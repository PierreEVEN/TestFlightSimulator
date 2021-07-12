#pragma once

#include "assets/asset_ptr.h"
#include "assimp/mesh.h"

#include <assimp/Importer.hpp>
#include <filesystem>
#include <memory>

class AssetManager;
class MeshData;

class MeshImporter
{
  public:
    MeshImporter(AssetManager* in_asset_manager) : asset_manager(in_asset_manager)
    {
        importer = std::make_unique<Assimp::Importer>();
    }

    TAssetPtr<MeshData> import_mesh(const std::filesystem::path& file_path, const std::string& asset_name, const std::string& desired_node);

    std::vector<std::string> get_mesh_list(const std::filesystem::path& file_path);

    static TAssetPtr<MeshData> process_mesh(const AssetId& asset_id, AssetManager* asset_manager, aiMesh* mesh, size_t id);

  private:
    std::unique_ptr<Assimp::Importer> importer;
    AssetManager*                     asset_manager;
};
