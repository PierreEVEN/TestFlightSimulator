
#include "assets/asset_mesh_data.h"

#include <string.h>

#include "engine_interface.h"
#include "rendering/vulkan/common.h"
#include "rendering/vulkan/utils.h"
#include "statsRecorder.h"

VkVertexInputBindingDescription Vertex::get_binding_description()
{
    VkVertexInputBindingDescription binding_description{};
    binding_description.binding   = 0;
    binding_description.stride    = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return binding_description;
}

std::vector<VkVertexInputAttributeDescription> Vertex::get_attribute_descriptions()
{
    std::vector<VkVertexInputAttributeDescription> attribute_description{};

    VkVertexInputAttributeDescription new_attribute;
    uint8_t                           currentLocation = 0;

    new_attribute.binding  = 0;
    new_attribute.location = currentLocation++;
    new_attribute.format   = VK_FORMAT_R32G32B32_SFLOAT;
    new_attribute.offset   = offsetof(Vertex, pos);
    attribute_description.push_back(new_attribute);

    new_attribute.binding  = 0;
    new_attribute.location = currentLocation++;
    new_attribute.format   = VK_FORMAT_R32G32_SFLOAT;
    new_attribute.offset   = offsetof(Vertex, uv);
    attribute_description.push_back(new_attribute);

    new_attribute.binding  = 0;
    new_attribute.location = currentLocation++;
    new_attribute.format   = VK_FORMAT_R32G32B32_SFLOAT;
    new_attribute.offset   = offsetof(Vertex, col);
    attribute_description.push_back(new_attribute);

    new_attribute.binding  = 0;
    new_attribute.location = currentLocation++;
    new_attribute.format   = VK_FORMAT_R32G32B32_SFLOAT;
    new_attribute.offset   = offsetof(Vertex, norm);
    attribute_description.push_back(new_attribute);

    new_attribute.binding  = 0;
    new_attribute.location = currentLocation++;
    new_attribute.format   = VK_FORMAT_R32G32B32_SFLOAT;
    new_attribute.offset   = offsetof(Vertex, tang);
    attribute_description.push_back(new_attribute);

    new_attribute.binding  = 0;
    new_attribute.location = currentLocation++;
    new_attribute.format   = VK_FORMAT_R32G32B32_SFLOAT;
    new_attribute.offset   = offsetof(Vertex, bitang);
    attribute_description.push_back(new_attribute);

    return attribute_description;
}

MeshData::MeshData(std::vector<Vertex> in_vertices, std::vector<uint32_t> in_indices) : vertices(std::move(in_vertices)), indices(std::move(in_indices))
{
    set_mesh_data(vertices, indices);
}

MeshData::~MeshData()
{
    if (vertex_buffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(get_engine_interface()->get_gfx_context()->vulkan_memory_allocator, vertex_buffer, vertex_buffer_allocation);
    if (index_buffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(get_engine_interface()->get_gfx_context()->vulkan_memory_allocator, index_buffer, index_buffer_allocation);
    vertex_buffer = VK_NULL_HANDLE;
    index_buffer  = VK_NULL_HANDLE;
}

void MeshData::set_mesh_data(const std::vector<Vertex>& in_vertices, const std::vector<uint32_t>& in_indices)
{
    if (indices.empty())
    {
        LOG_ERROR("Cannot create mesh : index buffer is empty");
        return;
    }
    if (vertices.empty())
    {
        LOG_ERROR("Cannot create mesh : vertex buffer is empty");
        return;
    }

    LOG_INFO("create static mesh %s", get_id().to_string().c_str());
    BEGIN_NAMED_RECORD(CREATE_MESH);

    void*          data;
    VkBuffer       staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    const VkDeviceSize vertex_buffer_size = sizeof(Vertex) * in_vertices.size();
    const VkDeviceSize index_buffer_size  = sizeof(uint32_t) * indices.size();

    /* Copy vertices */

    vulkan_utils::create_buffer(get_engine_interface()->get_gfx_context(), vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer,
                                staging_buffer_memory);

    vkMapMemory(get_engine_interface()->get_gfx_context()->logical_device, staging_buffer_memory, 0, vertex_buffer_size, 0, &data);
    memcpy(data, in_vertices.data(), static_cast<size_t>(vertex_buffer_size));
    vkUnmapMemory(get_engine_interface()->get_gfx_context()->logical_device, staging_buffer_memory);

    vulkan_utils::create_vma_buffer(get_engine_interface()->get_window(), vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer,
                                    vertex_buffer_allocation, vertex_buffer_alloc_info);

    vulkan_utils::copy_buffer(get_engine_interface()->get_window(), staging_buffer, vertex_buffer, vertex_buffer_size);

    vkDestroyBuffer(get_engine_interface()->get_gfx_context()->logical_device, staging_buffer, vulkan_common::allocation_callback);
    vkFreeMemory(get_engine_interface()->get_gfx_context()->logical_device, staging_buffer_memory, vulkan_common::allocation_callback);

    vulkan_utils::create_buffer(get_engine_interface()->get_gfx_context(), index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer,
                                staging_buffer_memory);

    /* Copy indices */

    vkMapMemory(get_engine_interface()->get_gfx_context()->logical_device, staging_buffer_memory, 0, index_buffer_size, 0, &data);
    memcpy(data, indices.data(), static_cast<size_t>(index_buffer_size));
    vkUnmapMemory(get_engine_interface()->get_gfx_context()->logical_device, staging_buffer_memory);

    vulkan_utils::create_vma_buffer(get_engine_interface()->get_window(), index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer,
                                    index_buffer_allocation, index_buffer_alloc_info);
    vulkan_utils::copy_buffer(get_engine_interface()->get_window(), staging_buffer, index_buffer, index_buffer_size);

    vkDestroyBuffer(get_engine_interface()->get_gfx_context()->logical_device, staging_buffer, vulkan_common::allocation_callback);
    vkFreeMemory(get_engine_interface()->get_gfx_context()->logical_device, staging_buffer_memory, vulkan_common::allocation_callback);
}
