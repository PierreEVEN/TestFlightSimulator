#pragma once
#include "assets/asset_ptr.h"
#include "node_primitive.h"

class MeshData;
class Material;

struct MeshProxyData
{
    Material* material;
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    size_t   indice_count;
};

class MeshNode : public PrimitiveNode
{
  public:
    MeshNode(TAssetPtr<MeshData> in_mesh, TAssetPtr<Material> in_material);
    
  private:
    TAssetPtr<MeshData> mesh;
    TAssetPtr<Material> material;

    EntityHandle proxy_entity_handle;
};