#pragma once
#include "assets/asset_ptr.h"
#include "node_primitive.h"

class MeshData;
class Material;

class MeshNode : public PrimitiveNode
{
  public:
    MeshNode(TAssetPtr<MeshData> in_mesh, TAssetPtr<Material> in_material) : mesh(in_mesh), material(in_material)
    {
    }

    void render(RenderContext render_context) override;

  private:
    TAssetPtr<MeshData> mesh;
    TAssetPtr<Material> material;
};