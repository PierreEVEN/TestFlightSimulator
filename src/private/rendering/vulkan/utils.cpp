#include "rendering/vulkan/utils.h"

#include <vector>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "config.h"
#include "ios/logger.h"

namespace vulkan_utils
{
	/*
	 * VALIDATION LAYERS
	 */
	
	VKAPI_ATTR VkBool32 VKAPI_CALL validation_layer_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
	{
		if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
			logger::log("VULKAN VALIDATION LAYER : %s", callback_data->pMessage);
		else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
			logger::validate("VULKAN VALIDATION LAYER : %s", callback_data->pMessage);
		else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			logger::warning("VULKAN VALIDATION LAYER : %s", callback_data->pMessage);
		else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			logger::fail("VULKAN VALIDATION LAYER : %s", callback_data->pMessage);
		}
		else
		{
			logger::error("VULKAN VALIDATION LAYER - UNKOWN VERBOSITY : %s", callback_data->pMessage);
		}

		return VK_FALSE;
	}
	
	VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_infos = {
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			nullptr,
			NULL,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
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
	
	queue_family_indices find_device_queue_families(VkSurfaceKHR surface, VkPhysicalDevice device)
	{
		queue_family_indices indices;

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
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport) {
				indices.present_family = i;
			}

			if (indices.is_complete()) {
				break;
			}

			i++;
		}
		return indices;
	}

	swapchain_support_details GetSwapchainSupportDetails(VkSurfaceKHR surface, VkPhysicalDevice device)
	{
		swapchain_support_details details;

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
		queue_family_indices indices = find_device_queue_families(surface, device);

		bool bAreExtensionSupported = CheckDeviceExtensionSupport(device);


		bool swapChainAdequate = false;
		if (bAreExtensionSupported) {
			swapchain_support_details swapChainSupport = GetSwapchainSupportDetails(surface, device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.is_complete() && bAreExtensionSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}
}
