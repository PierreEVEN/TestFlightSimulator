

#include "ios/scene_importer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include "assets/GraphicResource.h"
#include "assets/staticMesh.h"
#include "assets/texture2d.h"
#include "ios/logger.h"
#include "rendering/window.h"
#include "scene/node.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


SceneImporter::SceneImporter(Window* context, const std::filesystem::path& source_file)
	: window_context(context)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(source_file.string(), 
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (!scene) {
		logger_error("failed to import scene file %s", source_file.string().c_str());
		return;
	}

	object_name = scene->mName.data;

	// Generate resources
	for (size_t i = 0; i < scene->mNumTextures; ++i) meshes_refs[i] = process_texture(scene->mTextures[i]);
	for (size_t i = 0; i < scene->mNumMaterials; ++i) meshes_refs[i] = process_material(scene->mMaterials[i]);	
	for (size_t i = 0; i < scene->mNumMeshes; ++i) meshes_refs[i] = process_mesh(scene->mMeshes[i]);

	// Build scene
	process_node(scene->mRootNode, nullptr);
}

void SceneImporter::process_node(aiNode* ai_node, Node* parent)
{
	Node* node = create_node(ai_node, parent);
	
	for (const auto& ai_child : std::vector<aiNode*>(ai_node->mChildren, ai_node->mChildren + ai_node->mNumChildren)) process_node(ai_child, node);
}

Node* SceneImporter::create_node(aiNode* context, Node* parent)
{
	/*
	 * Extract transformation
	 */
	
	aiVector3t<float> ai_scale;
	aiVector3t<float> ai_pos;
	aiQuaternion ai_rot;
	context->mTransformation.Decompose(ai_scale, ai_rot, ai_pos);	
	const glm::dvec3 position(ai_pos.x, ai_pos.y, ai_pos.z);
	const glm::dquat rotation(ai_rot.x, ai_rot.y, ai_rot.z, ai_rot.w);
	const glm::dvec3 scale(ai_scale.x, ai_scale.y, ai_scale.z);

	logger_log("meshes : %s / meshes : %d", context->mName.data, context->mNumMeshes);

	
	Node* node = new Node(parent, position, rotation, scale);
	
	return node;
}


std::shared_ptr<AssetRef> SceneImporter::process_texture(aiTexture* texture)
{
	int width = texture->mWidth;
	int height = texture->mHeight;
	int channels = 4;
	uint8_t* data = nullptr;
	if (height == 0) {
		data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(texture->pcData), texture->mWidth, &width, &height, &channels, channels);
	}
	else
	{
		size_t pixel_count = texture->mWidth * texture->mHeight;
		data = new uint8_t[pixel_count * channels];
		for (size_t i = 0; i < pixel_count; ++i)
		{
			data[i * channels] = texture->pcData[i].r;
			if (channels > 1) data[i * channels + 1] = texture->pcData[i].g;
			if (channels > 2) data[i * channels + 2] = texture->pcData[i].b;
			if (channels > 3) data[i * channels + 3] = texture->pcData[i].a;
		}
	}
	
	std::shared_ptr<AssetRef> ref = std::make_shared<AssetRef>(object_name + "-mesh-" + texture->mFilename.data);

	GraphicResource::create<Texture2d>(window_context, *ref, data, width, height, channels);

	return ref;
}

std::shared_ptr<AssetRef> SceneImporter::process_material(aiMaterial* material)
{
	return nullptr;
}

std::shared_ptr<AssetRef> SceneImporter::process_mesh(aiMesh* mesh)
{
	VertexGroup vertex_group;

	// Get vertices
	vertex_group.pos.resize(mesh->mNumVertices);
	for (size_t i = 0; i < mesh->mNumVertices; ++i) {
		vertex_group.pos[i] = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
	}

	// Uvs
	if (mesh->HasTextureCoords(0))
	{
		vertex_group.uv.resize(mesh->mNumVertices);
		for (size_t i = 0; i < mesh->mNumVertices; ++i) {
			vertex_group.uv[i] = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
	}
	
	// Get normals
	if (mesh->HasNormals()) {
		vertex_group.norm.resize(mesh->mNumVertices);
		for (size_t i = 0; i < mesh->mNumVertices; ++i) {
			vertex_group.pos[i] = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		}
	}

	// Colors
	if (mesh->HasVertexColors(0)) {
		vertex_group.col.resize(mesh->mNumVertices);
		for (size_t i = 0; i < mesh->mNumVertices; ++i) {
			vertex_group.col[i] = glm::vec4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
		}
	}

	// Tangents
	if (mesh->HasTangentsAndBitangents()) 
	{
		vertex_group.tang.resize(mesh->mNumVertices);
		vertex_group.bitang.resize(mesh->mNumVertices);
		for (size_t i = 0; i < mesh->mNumVertices; ++i) {
			vertex_group.tang[i] = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			vertex_group.bitang[i] = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
		}
	}

	// Get triangles
	std::vector<uint32_t> triangles(mesh->mNumFaces * 3);
	
	for (size_t i = 0; i < mesh->mNumFaces; ++i)
	{
		if (mesh->mFaces[i].mNumIndices > 3)
		{
			logger_error("cannot process shapes that are not triangles : %s", mesh->mName.data);
			continue;
		}
		
		size_t face_index = i * 3;
		triangles[face_index] = mesh->mFaces[i].mIndices[0];
		triangles[face_index + 1] = mesh->mFaces[i].mIndices[1];
		triangles[face_index + 2] = mesh->mFaces[i].mIndices[2];
	}
	
	std::shared_ptr<AssetRef> ref = std::make_shared<AssetRef>(object_name + "-mesh-" + mesh->mName.data);
	
	GraphicResource::create<StaticMesh>(window_context, *ref, vertex_group, triangles);
	return ref;
}