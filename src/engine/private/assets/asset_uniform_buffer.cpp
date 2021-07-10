

#include "assets/asset_uniform_buffer.h"

#include "engine_interface.h"
#include "rendering/vulkan/utils.h"

#include <glm/glm.hpp>

UniformBuffer::UniformBuffer()
{
    uniformBuffers.resize(get_engine_interface()->get_window()->get_image_count());
    uniformBuffersMemory.resize(get_engine_interface()->get_window()->get_image_count());
    descriptorBufferInfo.resize(get_engine_interface()->get_window()->get_image_count());

    for (size_t i = 0; i < get_engine_interface()->get_window()->get_image_count(); i++)
    {
        vulkan_utils::create_buffer(get_engine_interface()->get_gfx_context(), data_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i],
                                    uniformBuffersMemory[i]);

        descriptorBufferInfo[i].buffer = uniformBuffers[i];
        descriptorBufferInfo[i].offset = 0;
        descriptorBufferInfo[i].range  = data_size;
    }
}

UniformBuffer::~UniformBuffer()
{
    for (size_t i = 0; i < uniformBuffers.size(); i++)
    {
        vkDestroyBuffer(get_engine_interface()->get_gfx_context()->logical_device, uniformBuffers[i], vulkan_common::allocation_callback);
        vkFreeMemory(get_engine_interface()->get_gfx_context()->logical_device, uniformBuffersMemory[i], vulkan_common::allocation_callback);
    }
}

VkDescriptorBufferInfo* UniformBuffer::get_descriptor_buffer_info(uint32_t image_index)
{
    // Create missing buffers for current image
    if (image_index >= descriptorBufferInfo.size())
    {
        uniformBuffers.resize(image_index + 1);
        uniformBuffersMemory.resize(image_index + 1);
        descriptorBufferInfo.resize(image_index + 1);
        dirty_buffers.resize(image_index + 1);
        
        for (size_t i = descriptorBufferInfo.size() - 1; i <= image_index; ++i)
        {
            vulkan_utils::create_buffer(get_engine_interface()->get_gfx_context(), data_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                        uniformBuffers[i], uniformBuffersMemory[i]);

            descriptorBufferInfo[i].buffer = uniformBuffers[i];
            descriptorBufferInfo[i].offset = 0;
            descriptorBufferInfo[i].range  = data_size;

            dirty_buffers[i] = true;
        }
    }

    if (dirty_buffers[image_index] || true)
    {
        void* memory_data;
        vkMapMemory(get_engine_interface()->get_gfx_context()->logical_device, uniformBuffersMemory[image_index], 0, sizeof(data_size), 0, &memory_data);
        memcpy(memory_data, data, sizeof(data_size));
        vkUnmapMemory(get_engine_interface()->get_gfx_context()->logical_device, uniformBuffersMemory[image_index]);
        dirty_buffers[image_index] = false;
    }
    
    return &descriptorBufferInfo[image_index];
}
