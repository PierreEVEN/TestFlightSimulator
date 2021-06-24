#pragma once

#include <cstdint>
#include <filesystem>
#include <vulkan/vulkan_core.h>



#include "assetBase.h"
#include "ui/imgui/imgui.h"


namespace job_system {
	class IJobTask;
}

class Texture2d : public AssetBase
{
public:
	Texture2d(const std::filesystem::path& texture_path);
	Texture2d(uint8_t* data, size_t width, size_t height, uint8_t channel_count);
	void create_image_descriptors();
	virtual ~Texture2d();
	
	ImTextureID get_texture_id(const size_t& image_index) { return static_cast<ImTextureID>(image_descriptors[image_index]); }

	bool try_load() override;

	[[nodiscard]] VkDescriptorImageInfo* get_descriptor_info();

private:

	void load_image();
	
	void create_image();
	void create_image_sampler();
	
	uint8_t* texture_data;
	int texture_width;
	int texture_height;
	int texture_channels;

	uint32_t mip_level;
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory image_memory = VK_NULL_HANDLE;
	VkImageView image_view = VK_NULL_HANDLE;
	VkSampler image_sampler = VK_NULL_HANDLE;
	VkDescriptorImageInfo* descriptor_image_info = nullptr;

	VkDescriptorSetLayout image_descriptor_layout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> image_descriptors;

	std::shared_ptr<job_system::IJobTask> creation_job;
};

