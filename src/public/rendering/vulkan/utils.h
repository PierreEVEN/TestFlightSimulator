#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vk_mem_alloc.h"

#include <optional>
#include <vector>

#include "ios/logger.h"
#define VK_ENSURE(condition, ...) if ((condition) != VK_SUCCESS) { logger_fail("VK_ERROR %d : %s", condition, __VA_ARGS__); }
#define VK_CHECK(object, ...) if ((object) == VK_NULL_HANDLE) { logger_fail("VK_ERROR_NULL_HANDLE %d : %s", object, __VA_ARGS__); }

namespace vulkan_utils
{

	class SwapchainSupportDetails {
	public:
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	
	class QueueFamilyIndices {
	public:
		QueueFamilyIndices() = default;
		std::optional<uint32_t> graphic_family;
		std::optional<uint32_t> transfert_family;
		std::optional<uint32_t> present_family;
		[[nodiscard]] bool is_complete() const { return graphic_family.has_value() && present_family.has_value(); }
	};
	
	extern VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_infos;

	std::vector<const char*> get_required_extensions();
	SwapchainSupportDetails get_swapchain_support_details(VkSurfaceKHR surface, VkPhysicalDevice device);
	QueueFamilyIndices find_device_queue_families(VkSurfaceKHR surface, VkPhysicalDevice device);
	bool is_physical_device_suitable(VkSurfaceKHR surface, VkPhysicalDevice device);
	VkSurfaceFormatKHR choose_swapchain_surface_format(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
	VkSampleCountFlagBits get_max_usable_sample_count(VkPhysicalDevice physical_device);
	VkPresentModeKHR choose_swapchain_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkFormat find_texture_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice physical_device);
	VkFormat get_depth_format(VkPhysicalDevice physical_device);
	VkExtent2D choose_swapchain_extend(const VkSurfaceCapabilitiesKHR& capabilities, const VkExtent2D& initial_extend);
	uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
}
