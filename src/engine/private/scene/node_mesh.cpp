

#include "scene/node_mesh.h"

#include "assets/asset_material.h"
#include "assets/asset_mesh.h"
#include "assets/asset_mesh_data.h"

void MeshNode::render(VkCommandBuffer& command_buffer, size_t image_index)
{
    if (!mesh)
        return;
    const auto&         mesh_data = mesh->get_mesh_data();
    TAssetPtr<Material> material  = nullptr;

    if (material_override)
    {
        material = material_override;
    }
    else
    {
        material = mesh->get_material();
    }
    if (!mesh_data || !material)
        return;


    /* Draw procedure : 
    vkCmdPushConstants(command_buffer, material->get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4f), &objectTransform);
    */

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->get_pipeline_layout(), 0, 1, &material->get_descriptor_sets()[image_index], 0, nullptr);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->get_pipeline());

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &mesh_data->get_vertex_buffer(), offsets);
    vkCmdBindIndexBuffer(command_buffer, mesh_data->get_index_buffer(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(command_buffer, mesh_data->get_indices_count(), 1, 0, 0, 0);
}
