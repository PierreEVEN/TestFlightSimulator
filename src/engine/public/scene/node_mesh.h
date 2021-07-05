#pragma once
#include "assets/asset_ptr.h"
#include "node_primitive.h"

class Material;
class Mesh;

class MeshNode : public PrimitiveNode
{
  public:
    MeshNode(TAssetPtr<Mesh> in_mesh, TAssetPtr<Material> in_material_override) : mesh(in_mesh), material_override(in_material_override)
    {
    }

    void render(RenderContext render_context) override;

  private:
    TAssetPtr<Mesh>     mesh;
    TAssetPtr<Material> material_override;
};