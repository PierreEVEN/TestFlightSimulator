#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>


#include "GraphicResource.h"
#include "ui/imgui/imgui.h"


class Texture2d : public GraphicResource
{
public:
	Texture2d(Window* context, const AssetRef& asset_reference, const uint8_t* data, size_t width, size_t height, uint8_t channel_count);
	void InitializeUIObjects();
	virtual ~Texture2d();
private:
	const uint8_t* texture_data;
	const size_t texture_width;
	const size_t texture_height;
	const size_t texture_channels;

	uint32_t mip_level;
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory image_memory = VK_NULL_HANDLE;
	VkImageView image_view = VK_NULL_HANDLE;
	VkSampler image_sampler = VK_NULL_HANDLE;

	VkDescriptorSetLayout ui_image_layout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> ui_image_descriptors;
};

