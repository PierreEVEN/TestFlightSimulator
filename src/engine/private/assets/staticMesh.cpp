

#include "assets/staticMesh.h"
#include <cstring>

#include "jobSystem/job_system.h"
#include "rendering/window.h"
#include "ui/window/windows/profiler.h"


VkVertexInputBindingDescription Vertex::get_binding_description()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Vertex::get_attribute_descriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

	VkVertexInputAttributeDescription newAttribute;
	uint8_t currentLocation = 0;
	
	newAttribute.binding = 0;
	newAttribute.location = currentLocation++;
	newAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	newAttribute.offset = offsetof(Vertex, pos);
	attributeDescriptions.push_back(newAttribute);
	
	newAttribute.binding = 0;
	newAttribute.location = currentLocation++;
	newAttribute.format = VK_FORMAT_R32G32_SFLOAT;
	newAttribute.offset = offsetof(Vertex, uv);
	attributeDescriptions.push_back(newAttribute);
	
	newAttribute.binding = 0;
	newAttribute.location = currentLocation++;
	newAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	newAttribute.offset = offsetof(Vertex, col);
	attributeDescriptions.push_back(newAttribute);
	
	newAttribute.binding = 0;
	newAttribute.location = currentLocation++;
	newAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	newAttribute.offset = offsetof(Vertex, norm);
	attributeDescriptions.push_back(newAttribute);
	
	newAttribute.binding = 0;
	newAttribute.location = currentLocation++;
	newAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	newAttribute.offset = offsetof(Vertex, tang);
	attributeDescriptions.push_back(newAttribute);
	
	newAttribute.binding = 0;
	newAttribute.location = currentLocation++;
	newAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	newAttribute.offset = offsetof(Vertex, bitang);
	attributeDescriptions.push_back(newAttribute);
	
	return attributeDescriptions;
}

StaticMesh::StaticMesh(const VertexGroup& in_vertices, const std::vector<uint32_t>& in_triangles)
	: vertices(in_vertices), indices(in_triangles)
{
	creation_job = job_system::new_job([&, in_vertices, in_triangles]
		{
        LOG_INFO("create static mesh %s", get_id().to_string().c_str());
			BEGIN_NAMED_RECORD(CREATE_MESH);
			void* data;
			VkBuffer stagin_buffer;
			VkDeviceMemory stagin_buffer_memory;

			VkDeviceSize vBufferSize = sizeof(Vertex) * vertices.vertices.size();
			VkDeviceSize iBufferSize = sizeof(uint32_t) * indices.size();
			
			/* Copy vertices */

			vulkan_utils::create_buffer(get_context()->get_context(), vBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagin_buffer, stagin_buffer_memory);

			vkMapMemory(get_context()->get_context()->logical_device, stagin_buffer_memory, 0, vBufferSize, 0, &data);
			memcpy(data, vertices.vertices.data(), (size_t)vBufferSize);
			vkUnmapMemory(get_context()->get_context()->logical_device, stagin_buffer_memory);

			vulkan_utils::create_vma_buffer(get_context(), vBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer, vertex_buffer_allocation, vertex_buffer_alloc_info);

			vulkan_utils::copy_buffer(get_context(), stagin_buffer, vertex_buffer, vBufferSize);
			
			vkDestroyBuffer(get_context()->get_context()->logical_device, stagin_buffer, vulkan_common::allocation_callback);
			vkFreeMemory(get_context()->get_context()->logical_device, stagin_buffer_memory, vulkan_common::allocation_callback);

			/* copy indices */
			if (indices.size() > 0) {
				vulkan_utils::create_buffer(get_context()->get_context(), iBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagin_buffer, stagin_buffer_memory);

				vkMapMemory(get_context()->get_context()->logical_device, stagin_buffer_memory, 0, iBufferSize, 0, &data);
				memcpy(data, indices.data(), (size_t)iBufferSize);
				vkUnmapMemory(get_context()->get_context()->logical_device, stagin_buffer_memory);

				vulkan_utils::create_vma_buffer(get_context(), iBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer, index_buffer_allocation, index_buffer_alloc_info);
				vulkan_utils::copy_buffer(get_context(), stagin_buffer, index_buffer, iBufferSize);

				vkDestroyBuffer(get_context()->get_context()->logical_device, stagin_buffer, vulkan_common::allocation_callback);
				vkFreeMemory(get_context()->get_context()->logical_device, stagin_buffer_memory, vulkan_common::allocation_callback);
			}
		});
}

StaticMesh::~StaticMesh()
{
	creation_job->wait();
	
	if (vertex_buffer != VK_NULL_HANDLE) vmaDestroyBuffer(get_context()->get_context()->vulkan_memory_allocator, vertex_buffer, vertex_buffer_allocation);
	if (index_buffer != VK_NULL_HANDLE) vmaDestroyBuffer(get_context()->get_context()->vulkan_memory_allocator, index_buffer, index_buffer_allocation);
	vertex_buffer = VK_NULL_HANDLE;
	index_buffer = VK_NULL_HANDLE;
}
