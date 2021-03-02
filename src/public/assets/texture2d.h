#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>


#include "GraphicResource.h"
#include "ui/imgui/imgui.h"


class Texture2d : public GraphicResource
{
public:
	Texture2d(Window* context, const AssetRef& asset_reference, uint8_t* data, size_t width, size_t height, uint8_t channel_count);
	void create_image_descriptors();
	virtual ~Texture2d();
	
	ImTextureID get_texture_id(const size_t& image_index) { return static_cast<ImTextureID>(image_descriptors[image_index]); }

private:

	void create_image();
	void create_image_sampler();
	
	uint8_t* texture_data;
	const size_t texture_width;
	const size_t texture_height;
	const size_t texture_channels;

	uint32_t mip_level;
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory image_memory = VK_NULL_HANDLE;
	VkImageView image_view = VK_NULL_HANDLE;
	VkSampler image_sampler = VK_NULL_HANDLE;

	VkDescriptorSetLayout image_descriptor_layout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> image_descriptors;
};

