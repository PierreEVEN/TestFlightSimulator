

#include "rendering/vulkan/swapchain.h"

#include <string>


#include "ios/logger.h"
#include "rendering/window.h"


Swapchain::Swapchain(const VkExtent2D& extend, Window* window)
		: surface_window(window)
{
	resize_swapchain(extend);
}

Swapchain::~Swapchain()
{
	destroy();
}

void Swapchain::resize_swapchain(VkExtent2D extend, const bool force_rebuild, const bool safe_resize)
{
	logger::log("resize swapchain ( %d x %d )", extend.width, extend.height);
	
	//if (safe_resize) extend = vulkan_utils::choose_swapchain_extend(surface_window->swapchain_support_details.capabilities, extend);

	if (extend.height != swapchain_extend.height || extend.height != swapchain_extend.width || force_rebuild)
	{
		swapchain_extend = extend;

		create_or_recreate();
	}
}

void Swapchain::create_or_recreate()
{
	if (swapchain != VK_NULL_HANDLE) destroy();

	std::string SwapChainLog = "Create new Swap chain :\n";
	
	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = surface_window->surface;
	create_info.minImageCount = surface_window->swapchain_image_count;
	create_info.imageFormat = surface_window->swapchain_surface_format.format;
	create_info.imageColorSpace = surface_window->swapchain_surface_format.colorSpace;

	//LOG_ASSERT(ToString((int)G_SWAPCHAIN_SURFACE_FORMAT.format));

	create_info.imageExtent = swapchain_extend;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	std::vector<uint32_t> queue_family_indices = { surface_window->context->queue_families.graphic_family.value(), surface_window->context->queue_families.present_family.value() };
	
	if (surface_window->context->queue_families.graphic_family != surface_window->context->queue_families.present_family) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size());
		create_info.pQueueFamilyIndices = queue_family_indices.data();
	}
	else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0; // Optional
		create_info.pQueueFamilyIndices = nullptr; // Optional
	}


	VkCompositeAlphaFlagBitsKHR alpha_composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	// Select the first composite alpha format available
	std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};
	for (auto& compositeAlphaFlag : compositeAlphaFlags) {
		if (surface_window->swapchain_support_details.capabilities.supportedCompositeAlpha & compositeAlphaFlag) {
			alpha_composite = compositeAlphaFlag;
			break;
		};
	}

	// Find the transformation of the surface
	VkSurfaceTransformFlagsKHR preTransform;
	if (surface_window->swapchain_support_details.capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		// We prefer a non-rotated transform
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surface_window->swapchain_support_details.capabilities.currentTransform;
	}

	create_info.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	create_info.compositeAlpha = alpha_composite;
	create_info.presentMode = surface_window->swapchain_present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	logger::log("create swapchain ( %d x %d ) / sharing mode : %d", swapchain_extend.width, swapchain_extend.height, create_info.imageSharingMode);
	VK_ENSURE(vkCreateSwapchainKHR(surface_window->context->logical_device, &create_info, vulkan_common::allocation_callback, &swapchain), "Failed to create swap chain");

	VK_CHECK(swapchain, "Invalid swapchain reference");

	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(swapchain_extend.width);
	viewport.height = static_cast<float>(swapchain_extend.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.extent = swapchain_extend;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
}

void Swapchain::destroy()
{
	logger::log("Destroy swapChain");	
	vkDestroySwapchainKHR(surface_window->context->logical_device, swapchain, vulkan_common::allocation_callback);
	swapchain = VK_NULL_HANDLE;
}
