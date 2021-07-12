
#include "ios/mesh_importer.h"


#include "assets/asset_mesh_data.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <cpputils/logger.hpp>

TAssetPtr<MeshData> MeshImporter::import_mesh(const std::string& asset_name, const std::filesystem::path& file_path, const std::string& desired_node)
{    
    if (!exists(file_path) || !is_regular_file(file_path))
    {
        LOG_ERROR("file %s doens't exists", file_path.string().c_str());
        return nullptr;
    }
    const aiScene* scene = importer->ReadFile(file_path.string(), aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

    if (!scene)
    {
        LOG_ERROR("failed to read file %s : %s", file_path.string().c_str(), importer->GetErrorString());
        return nullptr;
    }

    int asset_index = 0;
    while (asset_manager->find((asset_name + "_(" + std::to_string(asset_index) + ")").c_str()))
    {
        asset_index++;
    }


    for (size_t i = 0; i < scene->mNumMeshes; ++i)
    {
        if (scene->mMeshes[i]->mName.C_Str() == desired_node)
        {
            return process_mesh(asset_name, asset_manager, scene->mMeshes[i], i);
        }
    }

    LOG_WARNING("failed to find mesh %s in asset file %s", desired_node.c_str(), file_path.string().c_str());
    return nullptr;
}

std::vector<std::string> MeshImporter::get_mesh_list(const std::filesystem::path& file_path)
{
    if (!exists(file_path) || !is_regular_file(file_path))
    {
        LOG_ERROR("file %s doens't exists", file_path.string().c_str());
        return {};
    }
    const aiScene* scene = importer->ReadFile(file_path.string(), aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

    if (!scene)
    {
        LOG_ERROR("failed to read file %s : %s", file_path.string().c_str(), importer->GetErrorString());
        return {};
    }

    std::vector<std::string> mesh_list(scene->mNumMeshes);

    for (size_t i = 0; i < scene->mNumMeshes; ++i)
    {
        mesh_list[i] = scene->mMeshes[i]->mName.C_Str();
    }

    return mesh_list;
}


TAssetPtr<MeshData> MeshImporter::process_mesh(const std::string& asset_name, AssetManager* asset_manager, aiMesh* mesh, size_t id)
{
    std::vector<Vertex> vertex_group;

    vertex_group.resize(mesh->mNumVertices);
    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        vertex_group[i].pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

        if (mesh->HasTextureCoords(0))
            vertex_group[i].uv = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        if (mesh->HasNormals())
            vertex_group[i].norm = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        if (mesh->HasVertexColors(0))
            vertex_group[i].col = glm::vec4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
        if (mesh->HasTangentsAndBitangents())
        {
            vertex_group[i].tang   = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
            vertex_group[i].bitang = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        }
    }

    // Get triangles
    std::vector<uint32_t> triangles(mesh->mNumFaces * 3);

    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        if (mesh->mFaces[i].mNumIndices > 3)
        {
            LOG_ERROR("cannot process shapes that are not triangles : %s", mesh->mName.data);
            continue;
        }

        size_t face_index         = i * 3;
        triangles[face_index]     = mesh->mFaces[i].mIndices[0];
        triangles[face_index + 1] = mesh->mFaces[i].mIndices[1];
        triangles[face_index + 2] = mesh->mFaces[i].mIndices[2];
    }

    return asset_manager->create<MeshData>(asset_name.c_str(), vertex_group, triangles);
}