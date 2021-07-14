#pragma once

#include "assets/asset_ptr.h"
#include "assimp/mesh.h"

#include <assimp/Importer.hpp>
#include <filesystem>
#include <memory>

class MeshData;

class MeshImporter
{
  public:
    MeshImporter()
    {
        importer = std::make_unique<Assimp::Importer>();
    }

    TAssetPtr<MeshData> import_mesh(const std::filesystem::path& file_path, const std::string& asset_name, const std::string& desired_node);

    std::vector<std::string> get_mesh_list(const std::filesystem::path& file_path);

    static TAssetPtr<MeshData> process_mesh(const AssetId& asset_id, aiMesh* mesh, size_t id);

  private:
    std::unique_ptr<Assimp::Importer> importer;
};
