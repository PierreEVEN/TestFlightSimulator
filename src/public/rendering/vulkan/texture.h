#pragma once

#include "rendering/vulkan/utils.h"

class WindowContext;

namespace vulkan_texture
{
	void create_image_view_2d(WindowContext* context, VkImage image, VkImageView& view, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	
	void create_image_2d(WindowContext* context, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

}
