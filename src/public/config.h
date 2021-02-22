#pragma once
#include <vector>

namespace config {

	inline const size_t application_version_major = 1;
	inline const size_t application_version_minor = 0;
	inline const size_t application_version_patch = 0;

	inline const size_t engine_version_major = 1;
	inline const size_t engine_version_minor = 0;
	inline const size_t engine_version_patch = 0;
	
	inline const char* application_name = "Test flight simulator";
	inline const char* engine_name = "Test engine";

	/**
	 * Vulkan
	 */
	
	inline const bool use_validation_layers = true;
	inline const std::vector<const char*> required_validation_layers = { "VK_LAYER_KHRONOS_validation" };
	inline const std::vector<const char*> required_device_extensions = { "VK_KHR_swapchain" };

	
}
