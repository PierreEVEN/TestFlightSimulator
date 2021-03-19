#include "rendering/framegraph/framebuffer.h"


#include "rendering/window.h"
#include "rendering/vulkan/utils.h"


Framebuffer::Framebuffer(Window* context, const FramebufferDescription& framebuffer_description, VkExtent2D extend)
	:
	samples(framebuffer_description.samples),
	image_usage(framebuffer_description.is_depth_buffer ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
	image_format(framebuffer_description.format)
{
	is_depth_stencil_buffer = image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkImageAspectFlags aspectMask = 0;

	if (is_depth_stencil_buffer)
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else
	{
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkImageCreateInfo image{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = image_format,
		.extent = {
			.width = extend.width,
			.height = extend.height,
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = samples,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = image_usage | VK_IMAGE_USAGE_SAMPLED_BIT
	};

	VkMemoryRequirements memReqs;
	VK_ENSURE(vkCreateImage(context->get_context()->logical_device, &image, vulkan_common::allocation_callback, &buffer_image), "failed to create image");
	vkGetImageMemoryRequirements(context->get_context()->logical_device, buffer_image, &memReqs);

	VkMemoryAllocateInfo memAlloc = VkMemoryAllocateInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memReqs.size,
		.memoryTypeIndex = vulkan_utils::find_memory_type(context->get_context()->physical_device, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	VK_ENSURE(vkAllocateMemory(context->get_context()->logical_device, &memAlloc, vulkan_common::allocation_callback, &buffer_memory), "failed to allocate image memory");
	VK_ENSURE(vkBindImageMemory(context->get_context()->logical_device, buffer_image, buffer_memory, 0), "failed to bind image memory");

	VkImageViewCreateInfo view_info;
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = image_format;
	view_info.subresourceRange.aspectMask = aspectMask;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	view_info.image = buffer_image;
	view_info.flags = 0;
	view_info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view_info.pNext = nullptr;
	VK_ENSURE(vkCreateImageView(context->get_context()->logical_device, &view_info, vulkan_common::allocation_callback, &buffer_view), "failed to create image view");
}
