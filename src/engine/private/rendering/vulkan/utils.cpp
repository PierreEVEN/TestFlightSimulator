#include "rendering/vulkan/utils.h"

#include <vector>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "config.h"

#include <cpputils/logger.hpp>
#include "rendering/window.h"
#include "rendering/vulkan/commandPool.h"

namespace vulkan_utils
{
	/*
	 * VALIDATION LAYERS
	 */
	
	VKAPI_ATTR VkBool32 VKAPI_CALL validation_layer_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
	{
		if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
                LOG_INFO("VULKAN VALIDATION LAYER : %s", callback_data->pMessage);
		else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
                    LOG_VALIDATE("VULKAN VALIDATION LAYER : %s", callback_data->pMessage);
		else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                    LOG_WARNING("VULKAN VALIDATION LAYER : %s", callback_data->pMessage);
		else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
                    LOG_FATAL("VULKAN VALIDATION LAYER : %s", callback_data->pMessage);
		}
		else
		{
                    LOG_ERROR("VULKAN VALIDATION LAYER - UNKOWN VERBOSITY : %s", callback_data->pMessage);
		}

		return VK_FALSE;
	}
	
	VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_infos = {
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			nullptr,
			0,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			validation_layer_debug_callback,
	};
	
	std::vector<const char*> get_required_extensions()
	{
		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
		std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
		if (config::use_validation_layers) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		return extensions;
	}
	
	/*
	 * PHYSICAL DEVICE
	 */
	
	QueueFamilyIndices find_device_queue_families(VkSurfaceKHR surface, VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphic_family = i;
			}
			if (!indices.transfert_family.has_value() && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
				indices.transfert_family = i;
			}

			if (queueFamily.queueFlags == VK_QUEUE_TRANSFER_BIT) {
				indices.transfert_family = i;
			}
			VkBool32 presentSupport = false;
			VK_ENSURE(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport), "failed to get physical device present support");
			if (presentSupport) {
				indices.present_family = i;
			}

			if (indices.is_complete()) {
				break;
			}

			i++;
		}
		if (!indices.is_complete())
		{ LOG_FATAL("queue family indices are not complete");
		}
		return indices;
	}

	SwapchainSupportDetails get_swapchain_support_details(VkSurfaceKHR surface, VkPhysicalDevice device)
	{
		SwapchainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.present_modes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.present_modes.data());
		}

		return details;
	}
	
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		for (const auto& ext : config::required_device_extensions)
		{
			bool bContains = false;
			for (const auto& extension : availableExtensions) {
				if (!std::strcmp(ext, extension.extensionName)) bContains = true;
			}
			if (!bContains) return false;
		}

		return true;
	}

	bool is_physical_device_suitable(VkSurfaceKHR surface, VkPhysicalDevice device) {
		QueueFamilyIndices indices = find_device_queue_families(surface, device);

		bool bAreExtensionSupported = CheckDeviceExtensionSupport(device);


		bool swapChainAdequate = false;
		if (bAreExtensionSupported) {
			SwapchainSupportDetails swapChainSupport = get_swapchain_support_details(surface, device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.is_complete() && bAreExtensionSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}


	VkSurfaceFormatKHR choose_swapchain_surface_format(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
	{
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, NULL);
		assert(formatCount > 0);

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, surfaceFormats.data());

		VkSurfaceFormatKHR format;

		// If the surface log_format list only includes one entry with VK_FORMAT_UNDEFINED,
		// there is no preferered log_format, so we assume VK_FORMAT_B8G8R8A8_UNORM
		if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			format.format = VK_FORMAT_B8G8R8A8_UNORM;
			format.colorSpace = surfaceFormats[0].colorSpace;
		}
		else
		{
			// iterate over the list of available surface log_format and
			// check for the presence of VK_FORMAT_B8G8R8A8_UNORM
			bool found_B8G8R8A8_UNORM = false;
			for (auto&& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					format.format = surfaceFormat.format;
					format.colorSpace = surfaceFormat.colorSpace;
					found_B8G8R8A8_UNORM = true;
					break;
				}
			}

			// in case VK_FORMAT_B8G8R8A8_UNORM is not available
			// select the first available color log_format
			if (!found_B8G8R8A8_UNORM)
			{
				format.format = surfaceFormats[0].format;
				format.colorSpace = surfaceFormats[0].colorSpace;
			}
		}
		return format;
	}
	
	VkSampleCountFlagBits get_max_usable_sample_count(VkPhysicalDevice physical_device)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physical_device, &physicalDeviceProperties);

		const VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}

	VkPresentModeKHR choose_swapchain_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkFormat get_depth_format(VkPhysicalDevice physical_device)
	{
		return find_texture_format(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
			physical_device
		);
	}

	VkFormat find_texture_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
	                             VkFormatFeatureFlags features, VkPhysicalDevice physical_device)
	{
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);
			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		LOG_ERROR("cannot support required log_format");
		exit(EXIT_FAILURE);
	}

	VkExtent2D choose_swapchain_extend(const VkSurfaceCapabilitiesKHR& capabilities, const VkExtent2D& initial_extend)
	{
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			VkExtent2D actualExtent;

			actualExtent.width = std::clamp(initial_extend.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(initial_extend.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}


	uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		LOG_ERROR("Failed to find desired memory type");
		return -1;
	}


	VkCommandBuffer begin_single_time_commands(Window* context)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = context->get_command_pool();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(context->get_gfx_context()->logical_device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}

	void end_single_time_commands(Window* context, VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		VkFence submitFence = VK_NULL_HANDLE;
		vkCreateFence(context->get_gfx_context()->logical_device, &fenceInfo, vulkan_common::allocation_callback, &submitFence);

		vkResetFences(context->get_gfx_context()->logical_device, 1, &submitFence);
		{
			context->get_gfx_context()->submit_graphic_queue(submitInfo, submitFence);
		}
		vkWaitForFences(context->get_gfx_context()->logical_device, 1, &submitFence, VK_TRUE, UINT64_MAX);
		vkDestroyFence(context->get_gfx_context()->logical_device, submitFence, vulkan_common::allocation_callback);
		vkFreeCommandBuffers(context->get_gfx_context()->logical_device, context->get_command_pool(), 1, &commandBuffer);
	}


	void create_buffer(GfxContext* context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(context->logical_device, &bufferInfo, vulkan_common::allocation_callback, &buffer) != VK_SUCCESS) { LOG_FATAL("Failed to create buffer");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(context->logical_device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = find_memory_type(context->physical_device, memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(context->logical_device, &allocInfo, vulkan_common::allocation_callback, &bufferMemory) != VK_SUCCESS) { LOG_FATAL("Failled to allocate buffer memory");
		}

		vkBindBufferMemory(context->logical_device, buffer, bufferMemory, 0);
	}

	void copy_buffer(Window* context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		const VkCommandBuffer commandBuffer = begin_single_time_commands(context);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		end_single_time_commands(context, commandBuffer);
	}


	void create_vma_buffer(Window* context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VmaAllocation& allocation, VmaAllocationInfo& allocInfos)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo vmaInfos{};
		vmaInfos.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		vmaCreateBuffer(context->get_gfx_context()->vulkan_memory_allocator, &bufferInfo, &vmaInfos, &buffer, &allocation, &allocInfos);
	}

	
}
