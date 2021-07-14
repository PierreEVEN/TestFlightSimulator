#pragma once

#include <filesystem>

#include "assets/asset_ptr.h"
#include "assimp/Importer.hpp"

class Scene;
struct aiNode;
class MeshData;
class Node;

class Shader;

class SceneImporter final
{
  public:
    SceneImporter()
    {
        importer = std::make_unique<Assimp::Importer>();
    }
    ~SceneImporter() {}

    std::shared_ptr<Node> import_file(const std::filesystem::path& source_file, const std::string& asset_name, Scene* context_scene);

  private:
    //TAssetPtr<Texture2d> process_texture(struct aiTexture* texture, size_t id);
    TAssetPtr<Shader>    process_material(struct aiMaterial* material, size_t id);

    std::shared_ptr<Node> process_node(aiNode* ai_node, const std::shared_ptr<Node>& parent, Scene* context_scene);
    std::shared_ptr<Node> create_node(aiNode* context, const std::shared_ptr<Node>& parent, Scene* context_scene);
    
    std::string                       object_name;
    //std::vector<TAssetPtr<Texture2d>> texture_refs;
    std::vector<TAssetPtr<Shader>>    material_refs;
    std::vector<TAssetPtr<MeshData>>  meshes_refs;

    std::unique_ptr<Assimp::Importer> importer;
};