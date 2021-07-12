

#include "assets/asset_uniform_buffer.h"

#include "engine_interface.h"
#include "rendering/vulkan/utils.h"

ShaderBuffer::~ShaderBuffer()
{
    for (size_t i = 0; i < buffer.size(); i++)
    {
        vkDestroyBuffer(get_engine_interface()->get_gfx_context()->logical_device, buffer[i], vulkan_common::allocation_callback);
        vkFreeMemory(get_engine_interface()->get_gfx_context()->logical_device, buffer_memory[i], vulkan_common::allocation_callback);
    }
}

VkDescriptorBufferInfo* ShaderBuffer::get_descriptor_buffer_info(uint32_t image_index)
{
    // Create missing buffers for current image
    if (image_index >= descriptor_buffer_info.size())
    {
        buffer.resize(image_index + 1);
        buffer_memory.resize(image_index + 1);
        descriptor_buffer_info.resize(image_index + 1);
        dirty_buffers.resize(image_index + 1);

        for (size_t i = descriptor_buffer_info.size() - 1; i <= image_index; ++i)
        {
            vulkan_utils::create_buffer(get_engine_interface()->get_gfx_context(), data_size, buffer_usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer[i], buffer_memory[i]);

            descriptor_buffer_info[i].buffer = buffer[i];
            descriptor_buffer_info[i].offset = 0;
            descriptor_buffer_info[i].range  = data_size;

            dirty_buffers[i] = true;
        }
    }

    if (dirty_buffers[image_index])
    {
        void* memory_data;
        vkMapMemory(get_engine_interface()->get_gfx_context()->logical_device, buffer_memory[image_index], 0, data_size, 0, &memory_data);
        memcpy(memory_data, data, data_size);
        vkUnmapMemory(get_engine_interface()->get_gfx_context()->logical_device, buffer_memory[image_index]);
        dirty_buffers[image_index] = false;
    }

    return &descriptor_buffer_info[image_index];
}
