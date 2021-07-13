

#include "scene/node_mesh.h"

#include "assets/asset_material.h"
#include "assets/asset_mesh_data.h"

MeshNode::MeshNode(TAssetPtr<MeshData> in_mesh, TAssetPtr<Material> in_material) : mesh(in_mesh), material(in_material)
{
    if (!material)
    {
        LOG_ERROR("material is not valid");
        return;
    }

    proxy_entity_handle = get_render_scene()->get_scene_proxy().add_entity(MeshProxyData{
        .material      = static_cast<Material*>(in_material.get()),
        .vertex_buffer = in_mesh->get_vertex_buffer(),
        .index_buffer  = in_mesh->get_index_buffer(),
        .indice_count  = in_mesh->get_indices_count(),
    });
}