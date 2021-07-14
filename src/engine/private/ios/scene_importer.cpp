
#include "ios/scene_importer.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "rendering/window.h"
#include <cpputils/logger.hpp>

#include "engine_interface.h"
#include "ios/mesh_importer.h"
#include "jobSystem/job_system.h"
#include "scene/node_base.h"
#include "scene/scene.h"
#include "stb_image.h"
#include "scene/node_mesh.h"
#include "ui/window/windows/profiler.h"
#include "assets/asset_mesh.h"

std::shared_ptr<Node> SceneImporter::process_node(aiNode* ai_node, const std::shared_ptr<Node>& parent, Scene* context_scene)
{
    auto base_node = create_node(ai_node, parent, context_scene);

    for (const auto& ai_child : std::vector<aiNode*>(ai_node->mChildren, ai_node->mChildren + ai_node->mNumChildren))
    {
        auto container_node = process_node(ai_child, base_node, context_scene);
        container_node->attach_to(base_node);
    }

    return base_node;
}

std::shared_ptr<Node> SceneImporter::create_node(aiNode* context, const std::shared_ptr<Node>& parent, Scene* context_scene)
{
    // Extract transformation
    aiVector3t<float> ai_scale;
    aiVector3t<float> ai_pos;
    aiQuaternion      ai_rot;
    context->mTransformation.Decompose(ai_scale, ai_rot, ai_pos);
    const glm::dvec3 position(ai_pos.x, ai_pos.y, ai_pos.z);
    const glm::dquat rotation(ai_rot.x, ai_rot.y, ai_rot.z, ai_rot.w);
    const glm::dvec3 scale(ai_scale.x, ai_scale.y, ai_scale.z);

    auto node = context_scene->add_node<Node>();
    if (parent)
        node->attach_to(parent);

    for (size_t i = 0; i < context->mNumMeshes; ++i)
    {
        auto mesh_node = context_scene->add_node<MeshNode>(meshes_refs[context->mMeshes[i]], TAssetPtr<Material>("test_material"));
        mesh_node->attach_to(node);
    }

    LOG_INFO("meshes : %s / meshes : %d", context->mName.data, context->mNumMeshes);

    return node;
}

std::shared_ptr<Node> SceneImporter::import_file(const std::filesystem::path& source_file, const std::string& asset_name, Scene* context_scene)
{
    BEGIN_NAMED_RECORD(IMPORT_SCENE_DATA);
    if (!exists(source_file) || !is_regular_file(source_file))
    {
        LOG_ERROR("file %s doens't exists", source_file.string().c_str());
        return nullptr;
    }
    const aiScene* scene = importer->ReadFile(source_file.string(), aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

    if (!scene)
    {
        LOG_ERROR("failed to import scene file %s : %s", source_file.string().c_str(), importer->GetErrorString());
        return nullptr;
    }

    if (object_name.empty())
        object_name = scene->mName.data;

    // Generate resources
    // texture_refs.resize(scene->mNumTextures);
    // material_refs.resize(scene->mNumMaterials);
    meshes_refs.resize(scene->mNumMeshes);
    /*
    for (size_t i = 0; i < scene->mNumTextures && i < 1; ++i)
        texture_refs[i] = process_texture(scene->mTextures[i], i);
        */
    /*
    for (size_t i = 0; i < scene->mNumMaterials; ++i)
        material_refs[i] = process_material(scene->mMaterials[i], i);
        */
    for (size_t i = 0; i < scene->mNumMeshes; ++i)
        meshes_refs[i] = MeshImporter::process_mesh(AssetManager::get()->find_valid_asset_id(asset_name + "_" + scene->mMeshes[i]->mName.C_Str()), scene->mMeshes[i], i);


    auto root_node = process_node(scene->mRootNode, nullptr, context_scene);
    return root_node;
}

/*
TAssetPtr<Texture2d> SceneImporter::process_texture(aiTexture* texture, size_t id)
{
    int      width    = static_cast<int>(texture->mWidth);
    int      height   = static_cast<int>(texture->mHeight);
    int      channels = 4;
    uint8_t* data     = nullptr;
    if (height == 0) { data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(texture->pcData), texture->mWidth, &width, &height, &channels, channels); }
    else
    {
        size_t pixel_count = texture->mWidth * texture->mHeight;
        data               = new uint8_t[pixel_count * channels];
        for (size_t i = 0; i < pixel_count; ++i)
        {
            data[i * channels] = texture->pcData[i].r;
            if (channels > 1) data[i * channels + 1] = texture->pcData[i].g;
            if (channels > 2) data[i * channels + 2] = texture->pcData[i].b;
            if (channels > 3) data[i * channels + 3] = texture->pcData[i].a;
        }
    }

    AssetId              asset_id(object_name + "-texture-" + texture->mFilename.data + "_" + std::to_string(id));
    TAssetPtr<Texture2d> asset = asset_manager->create<Texture2d>(asset_id, data, width, height, channels);

    return asset;
}
*/

TAssetPtr<Shader> SceneImporter::process_material(aiMaterial* material, size_t id)
{
    return TAssetPtr<Shader>();
}