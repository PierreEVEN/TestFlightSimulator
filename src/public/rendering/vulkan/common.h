#pragma once

#include <vector>
#include <optional>

#include "rendering/vulkan/utils.h"

namespace vulkan_common {

	void vulkan_init();
	
	void create_instance();
	void create_validation_layers();

	
	void close();

	inline VkAllocationCallbacks* allocation_callback = nullptr;
	inline VkInstance instance;
}