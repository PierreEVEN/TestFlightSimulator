#pragma once
#include "asset_base.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <string.h>

class UniformBuffer : public AssetBase
{
  public:
    UniformBuffer();
    ~UniformBuffer() override;

    template <typename Struct_T> UniformBuffer(std::string in_buffer_name, const Struct_T& in_data) : buffer_name(in_buffer_name)
    {
        set_data<Struct_T>(in_data);
    }

    template <typename Struct_T> void set_data(const Struct_T& in_data)
    {
        if (data_size != sizeof(Struct_T))
        {
            data_size = sizeof(Struct_T);
            data      = realloc(data, data_size);
        }
        memcpy(data, &in_data, data_size);
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

    size_t            data_size    = 0;
    void*             data         = nullptr;
    std::string       buffer_name;

    std::vector<VkDescriptorBufferInfo> descriptor_buffer_info;
    std::vector<VkBuffer>               buffer;
    std::vector<VkDeviceMemory>         buffer_memory;
    std::vector<bool>                   dirty_buffers;

    VkWriteDescriptorSet write_descriptor_set;
};
