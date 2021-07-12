#pragma once
#include "asset_base.h"

#include <glm/glm.hpp>
#include <string.h>
#include <vulkan/vulkan.h>

class ShaderBuffer : public AssetBase
{
  public:
    ShaderBuffer(std::string in_buffer_name, size_t data_size, VkBufferUsageFlags in_buffer_usage) : buffer_name(in_buffer_name), buffer_usage(in_buffer_usage)
    {
        resize_buffer(data_size);
    }

    template <typename Struct_T> ShaderBuffer(std::string in_buffer_name, const Struct_T& in_data, VkBufferUsageFlags in_buffer_usage) : buffer_name(in_buffer_name), buffer_usage(in_buffer_usage)
    {
        set_data<Struct_T>(in_data);
    }
    ~ShaderBuffer() override;

    template <typename Struct_T> void set_data(const Struct_T& in_data)
    {
        resize_buffer(sizeof(Struct_T));
        write_buffer(&in_data, sizeof(in_data));
    }

    void resize_buffer(size_t in_data_size)
    {
        if (data_size != in_data_size)
        {
            data_size = in_data_size;
            data      = realloc(data, data_size);
        }
    }

    void write_buffer(const void* in_data, size_t in_size, size_t in_offset = 0)
    {
        if (in_size + in_offset > data_size)
        {
            LOG_WARNING("trying to write out of buffer range");
        }
        
        if (in_data)
            memcpy(static_cast<char*>(data) + in_offset, in_data, data_size);

        for (int i = 0; i < dirty_buffers.size(); ++i)
        {
            dirty_buffers[i] = true;
        }
    }

    [[nodiscard]] std::string get_name() const
    {
        return buffer_name;
    }

    [[nodiscard]] VkDescriptorBufferInfo* get_descriptor_buffer_info(uint32_t image_index);

  private:
    struct CameraData2
    {
        glm::mat4 world_projection = glm::mat4(1.0);
        glm::mat4 view_matrix      = glm::mat4(1.0);
        glm::vec3 camera_location  = glm::vec3(0, 0, 0);
    };

    size_t             data_size = 0;
    void*              data      = nullptr;
    std::string        buffer_name;
    VkBufferUsageFlags buffer_usage;

    std::vector<VkDescriptorBufferInfo> descriptor_buffer_info;
    std::vector<VkBuffer>               buffer;
    std::vector<VkDeviceMemory>         buffer_memory;
    std::vector<bool>                   dirty_buffers;

    VkWriteDescriptorSet write_descriptor_set;
};
