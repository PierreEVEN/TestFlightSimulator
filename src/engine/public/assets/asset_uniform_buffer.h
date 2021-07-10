#pragma once
#include "asset_base.h"
#include <vulkan/vulkan.h>

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
        data_size = sizeof(Struct_T);
        data      = realloc(data, data_size);
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
    size_t            data_size    = 0;
    void*             data         = nullptr;
    std::string       buffer_name;

    std::vector<VkDescriptorBufferInfo> descriptorBufferInfo;
    std::vector<VkBuffer>               uniformBuffers;
    std::vector<VkDeviceMemory>         uniformBuffersMemory;
    std::vector<bool>                   dirty_buffers;

    VkWriteDescriptorSet write_descriptor_set;
};
