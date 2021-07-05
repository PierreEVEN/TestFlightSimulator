#pragma once

#include "asset_base.h"

class MeshData;
class Material;

class Mesh : public AssetBase
{
  public:
    Mesh(const TAssetPtr<MeshData>& in_mesh_data, const TAssetPtr<Material>& in_mesh_material) : mesh_data(in_mesh_data), mesh_material(in_mesh_material)
    {
    }

    const TAssetPtr<MeshData>& get_mesh_data() const
    {
        return mesh_data;
    }

    const TAssetPtr<Material>& get_material() const
    {
        return mesh_material;
    }

  private:
    TAssetPtr<MeshData> mesh_data;
    TAssetPtr<Material> mesh_material;
};
