#include "rendering/vulkan/texture.h"


#include "rendering/window.h"
#include "rendering/vulkan/common.h"

namespace vulkan_texture {

	void create_image_view_2d(WindowContext* context, VkImage image, VkImageView& view, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.components = { VK_COMPONENT_SWIZZLE_R,	VK_COMPONENT_SWIZZLE_G,	VK_COMPONENT_SWIZZLE_B,	VK_COMPONENT_SWIZZLE_A };
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.subresourceRange.levelCount = mipLevels;

		VK_ENSURE(vkCreateImageView(context->logical_device, &viewInfo, vulkan_common::allocation_callback, &view), "Failed to create view on texture image");
	}

	void create_image_2d(WindowContext* context, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
		VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
		VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = numSamples;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_ENSURE(vkCreateImage(context->logical_device, &imageInfo, vulkan_common::allocation_callback, &image), "failed to create image");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(context->logical_device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = vulkan_utils::find_memory_type(context->physical_device, memRequirements.memoryTypeBits, properties);

		VK_ENSURE(vkAllocateMemory(context->logical_device, &allocInfo, vulkan_common::allocation_callback, &imageMemory), "failed to allocate image memory");

		vkBindImageMemory(context->logical_device, image, imageMemory, 0);
	}
}
