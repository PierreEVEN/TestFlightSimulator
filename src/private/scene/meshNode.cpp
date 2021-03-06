
#include "scene/meshNode.h"


#include "assets/shader.h"
#include "assets/staticMesh.h"

void MeshNode::draw(VkCommandBuffer buffer, uint8_t image_index)
{
	if (!mesh.get()) return;
	if (!material.get()) return;

	bind_pipeline(buffer, image_index);
	bind_mesh(buffer);

}

void MeshNode::bind_pipeline(VkCommandBuffer buffer, uint8_t image_index)
{
	vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->get_pipeline_layout(), 0, 1, &material->get_descriptor_set(image_index), 0, nullptr);
	vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->get_pipeline());
}

void MeshNode::bind_mesh(VkCommandBuffer buffer)
{
	VkBuffer vertexBuffers[] = { mesh->get_vertex_buffer() };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);

	if (mesh->get_indices_count() > 0)
	{
		vkCmdBindIndexBuffer(buffer, mesh->get_index_buffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(buffer, static_cast<uint32_t>(mesh->get_indices_count()), 1, 0, 0, 0);
	}
	else {
		vkCmdDraw(buffer, static_cast<uint32_t>(mesh->get_vertices_count()), 1, 0, 0);
	}
}
