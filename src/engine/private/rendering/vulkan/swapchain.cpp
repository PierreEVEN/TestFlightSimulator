

#include "rendering/vulkan/swapchain.h"

#include <string>



#include "config.h"
#include "ios/logger.h"
#include "rendering/window.h"


Swapchain::Swapchain(const VkExtent2D& extend, Window* window)
		: surface_window(window)
{
	create_fences_and_semaphores();
	set_size(extend);
}

Swapchain::~Swapchain()
{
	destroy();
}

void Swapchain::set_size(VkExtent2D extend, const bool force_rebuild, const bool safe_resize)
{
	//if (safe_resize) extend = vulkan_utils::choose_swapchain_extend(surface_window->SwapchainSupportDetails.capabilities, extend);

	if (extend.height != swapchain_extend.height || extend.width != swapchain_extend.width || force_rebuild)
	{
		logger_log("resize swapchain ( %d x %d )", extend.width, extend.height);

		swapchain_extend = extend;

		create_or_recreate();
	}
}

void Swapchain::create_fences_and_semaphores()
{
	logger_log("create fence and semaphores\n\t-in flight fence : %d\n\t-images in flight : %d", config::max_frame_in_flight, surface_window->get_image_count());
	image_acquire_semaphore.resize(config::max_frame_in_flight);
	in_flight_fences.resize(config::max_frame_in_flight);
	images_in_flight.resize(surface_window->get_image_count(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < config::max_frame_in_flight; i++) {
		VK_ENSURE(vkCreateSemaphore(surface_window->get_context()->logical_device, &semaphoreInfo, vulkan_common::allocation_callback, &image_acquire_semaphore[i]), "Failed to create image available semaphore #%d" + i);
		VK_ENSURE(vkCreateFence(surface_window->get_context()->logical_device, &fenceInfo, vulkan_common::allocation_callback, &in_flight_fences[i]), "Failed to create fence #%d" + i);
	}
}

DrawInfo Swapchain::acquire_next_image()
{
	// Select next image (default : [0 : 1])
	image_in_flight = (image_in_flight + 1) % config::max_frame_in_flight;

	// Synchronise CPU over GPU
	vkWaitForFences(surface_window->get_context()->logical_device, 1, &in_flight_fences[image_in_flight], VK_TRUE, UINT64_MAX);

	// Retrieve the next available image ID (default : [0 - 2])
	uint32_t image_index;
	const VkResult result = vkAcquireNextImageKHR(surface_window->get_context()->logical_device, swapchain, UINT64_MAX, image_acquire_semaphore[image_in_flight], VK_NULL_HANDLE, &image_index);

	// If KHR is outdated, try to resize swapchain
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		int width = 0, height = 0;
		glfwGetFramebufferSize(surface_window->get_handle(), &width, &height);
		surface_window->resize_window(width, height);
		return acquire_next_image(); // Try to acquire next image when finished
	}
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		logger_fail("Failed to acquire image from the swapchain");
	}

	if (images_in_flight[image_index] != VK_NULL_HANDLE) vkWaitForFences(surface_window->get_context()->logical_device, 1, &images_in_flight[image_index], VK_TRUE, UINT64_MAX);
	images_in_flight[image_index] = in_flight_fences[image_in_flight];

	return DrawInfo{
		.current_pass = nullptr,
		.context = surface_window,
		.command_buffer = VK_NULL_HANDLE,
		.image_index = image_index,
		.frame_id = image_in_flight,
	};
}

void Swapchain::submit_next_image(uint32_t image_index, std::vector<VkSemaphore> render_finished_semaphores)
{
	/**
	 * Present to swapchain
	 */

	const VkPresentInfoKHR present_infos{	
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = static_cast<uint32_t>(render_finished_semaphores.size()),
		.pWaitSemaphores = render_finished_semaphores.data(),
		.swapchainCount = 1,
		.pSwapchains = &swapchain,
		.pImageIndices = &image_index,
		.pResults = nullptr,
	};
	const VkResult result = surface_window->get_context()->submit_present_queue(present_infos);
	
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || surface_window->bHasViewportBeenResized) {
		int width = 0;
		int height = 0;
		glfwGetFramebufferSize(surface_window->get_handle(), &width, &height);
		surface_window->resize_window(width, height);
	}
	else if (result != VK_SUCCESS) {
		logger_fail("Failed to present image to swap chain");
	}	
}

void Swapchain::create_or_recreate()
{
	if (swapchain != VK_NULL_HANDLE) destroy();

	std::string SwapChainLog = "Create new Swap chain :\n";
	
	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = surface_window->get_surface();
	create_info.minImageCount = surface_window->get_image_count();
	create_info.imageFormat = surface_window->get_surface_format().format;
	create_info.imageColorSpace = surface_window->get_surface_format().colorSpace;

	//LOG_ASSERT(ToString((int)G_SWAPCHAIN_SURFACE_FORMAT.log_format));

	create_info.imageExtent = swapchain_extend;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	std::vector<uint32_t> queue_family_indices = { surface_window->get_context()->queue_families.graphic_family.value(), surface_window->get_context()->queue_families.present_family.value() };
	
	if (surface_window->get_context()->queue_families.graphic_family != surface_window->get_context()->queue_families.present_family) {
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
	// Select the first composite alpha log_format available
	std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};
	for (auto& compositeAlphaFlag : compositeAlphaFlags) {
		if (surface_window->get_support_details().capabilities.supportedCompositeAlpha & compositeAlphaFlag) {
			alpha_composite = compositeAlphaFlag;
			break;
		}
	}

	// Find the transformation of the surface
	VkSurfaceTransformFlagsKHR preTransform;
	if (surface_window->get_support_details().capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		// We prefer a non-rotated transform
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surface_window->get_support_details().capabilities.currentTransform;
	}

	create_info.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	create_info.compositeAlpha = alpha_composite;
	create_info.presentMode = surface_window->get_present_mode();
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	logger_log("create swapchain ( %d x %d ) / sharing mode : %d", swapchain_extend.width, swapchain_extend.height, create_info.imageSharingMode);

	VK_ENSURE(vkCreateSwapchainKHR(surface_window->get_context()->logical_device, &create_info, vulkan_common::allocation_callback, &swapchain), "Failed to create swap chain");
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
	logger_log("Destroy swapChain");
	vkDestroySwapchainKHR(surface_window->get_context()->logical_device, swapchain, vulkan_common::allocation_callback);
	swapchain = VK_NULL_HANDLE;
}
