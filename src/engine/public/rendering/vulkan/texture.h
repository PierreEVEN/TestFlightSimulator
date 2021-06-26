#pragma once

#include "rendering/vulkan/utils.h"

class GfxContext;

namespace vulkan_texture
{
	void create_image_view_2d(GfxContext* context, VkImage image, VkImageView& view, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	
	void create_image_2d(GfxContext* context, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	void transition_image_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevel, VkCommandBuffer commandBuffer);

	void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkCommandBuffer commandBuffer);
	
	void generate_mipmaps(Window* context, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, VkCommandBuffer commandBuffer);
}
