#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vk_mem_alloc.h"

#include <optional>
#include <vector>


#define VK_ENSURE(condition, message) if ((condition) != VK_SUCCESS) { logger::fail("VK_ERROR : %s", message); }
#define VK_CHECK(object, message) if ((object) == VK_NULL_HANDLE) { logger::fail("VK_ERROR_NULL_HANDLE : %s", message); }

namespace vulkan_utils
{

	struct swapchain_support_details {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	
	struct queue_family_indices {
		queue_family_indices() = default;
		std::optional<uint32_t> graphic_family;
		std::optional<uint32_t> transfert_family;
		std::optional<uint32_t> present_family;
		[[nodiscard]] bool is_complete() const { return graphic_family.has_value() && present_family.has_value(); }
	};
	
	extern VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_infos;

	std::vector<const char*> get_required_extensions();
	
	queue_family_indices find_device_queue_families(VkSurfaceKHR surface, VkPhysicalDevice device);

	bool is_physical_device_suitable(VkSurfaceKHR surface, VkPhysicalDevice device);
	
}
