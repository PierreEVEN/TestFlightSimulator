

#include "rendering/vulkan/framebuffer.h"



#include <cpputils/logger.hpp>
#include "rendering/window.h"
#include "rendering/vulkan/swapchain.h"
#include "rendering/vulkan/texture.h"

Framebuffer::Framebuffer(Window* window_context, VkExtent2D size)
	: window(window_context)
{
	LOG_INFO("create framebuffer ( %d x %d )", size.width, size.height);
	set_size(size, true);
}

Framebuffer::~Framebuffer()
{
    LOG_INFO("destroy framebuffer");
	delete swapchain;
	destroy_framebuffer();
	destroy_framebuffer_images();
}

void Framebuffer::set_size(VkExtent2D size, bool force_rebuild)
{
	if (size.width != buffer_size.width || size.height != buffer_size.height || force_rebuild)
	{
		if (!swapchain) swapchain = new Swapchain(size, window);
		swapchain->set_size(size);
		buffer_size = size;

		if (color_target != VK_NULL_HANDLE)
		{
			destroy_framebuffer();
			destroy_framebuffer_images();
		}

		create_framebuffer_images();
		create_framebuffer();		
	}	
}

void Framebuffer::create_framebuffer_images()
{
	/** Color buffer */
	VkFormat colorFormat = window->get_surface_format().format;
	vulkan_texture::create_image_2d(window->get_gfx_context(), buffer_size.width, buffer_size.height, 1, static_cast<VkSampleCountFlagBits>(window->get_msaa_sample_count()), colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, color_target, color_memory);
	vulkan_texture::create_image_view_2d(window->get_gfx_context(), color_target, color_view, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	/** Depth buffer */
	VkFormat depthFormat = vulkan_utils::get_depth_format(window->get_gfx_context()->physical_device);
	vulkan_texture::create_image_2d(window->get_gfx_context(), buffer_size.width, buffer_size.height, 1, static_cast<VkSampleCountFlagBits>(window->get_msaa_sample_count()), depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_target, depth_memory);
	vulkan_texture::create_image_view_2d(window->get_gfx_context(), depth_target, depth_view, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	/** Swap chain buffer */
	uint32_t swapchain_image_count = window->get_image_count();
	swapchain_images.resize(window->get_image_count());
	swapchain_image_views.resize(window->get_image_count());
	vkGetSwapchainImagesKHR(window->get_gfx_context()->logical_device, swapchain->get(), &swapchain_image_count, nullptr);
	vkGetSwapchainImagesKHR(window->get_gfx_context()->logical_device, swapchain->get(), &swapchain_image_count, swapchain_images.data());
	for (size_t i = 0; i < swapchain_images.size(); i++)
	{
		vulkan_texture::create_image_view_2d(window->get_gfx_context(), swapchain_images[i], swapchain_image_views[i], window->get_surface_format().format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

void Framebuffer::create_framebuffer()
{
	framebuffers.resize(window->get_image_count());

	for (size_t i = 0; i < window->get_image_count(); i++) {
		std::vector<VkImageView> attachments;
		if (window->get_msaa_sample_count() > 1)
		{
			attachments.push_back(color_view);
			attachments.push_back(depth_view);
			attachments.push_back(swapchain_image_views[i]);
		}
		else
		{
			attachments.push_back(swapchain_image_views[i]);
			attachments.push_back(depth_view);
		}

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = window->get_render_pass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = buffer_size.width;
		framebufferInfo.height = buffer_size.height;
		framebufferInfo.layers = 1;

		VK_ENSURE(vkCreateFramebuffer(window->get_gfx_context()->logical_device, &framebufferInfo, vulkan_common::allocation_callback, &framebuffers[i]), "Failed to create framebuffer #%d" + i);
	}
}

void Framebuffer::destroy_framebuffer_images()
{
	for (auto framebuffer : framebuffers) {
		vkDestroyFramebuffer(window->get_gfx_context()->logical_device, framebuffer, nullptr);
	}
}

void Framebuffer::destroy_framebuffer()
{
	/** Color buffer */
	vkDestroyImageView(window->get_gfx_context()->logical_device, color_view, vulkan_common::allocation_callback);
	vkDestroyImage(window->get_gfx_context()->logical_device, color_target, vulkan_common::allocation_callback);
	vkFreeMemory(window->get_gfx_context()->logical_device, color_memory, vulkan_common::allocation_callback);
	color_view = VK_NULL_HANDLE;
	color_target = VK_NULL_HANDLE;
	color_memory = VK_NULL_HANDLE;
	
	/** Depth buffer */
	vkDestroyImageView(window->get_gfx_context()->logical_device, depth_view, vulkan_common::allocation_callback);
	vkDestroyImage(window->get_gfx_context()->logical_device, depth_target, vulkan_common::allocation_callback);
	vkFreeMemory(window->get_gfx_context()->logical_device, depth_memory, vulkan_common::allocation_callback);
	depth_view = VK_NULL_HANDLE;
	depth_target = VK_NULL_HANDLE;
	depth_memory = VK_NULL_HANDLE;

	/* Swap chain buffers */
	for (auto imageView : swapchain_image_views)
		vkDestroyImageView(window->get_gfx_context()->logical_device, imageView, vulkan_common::allocation_callback);
	
	swapchain_image_views.clear();
}
