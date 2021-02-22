#include "rendering/vulkan/common.h"



#include <set>
#include <vector>
#include <vulkan/vulkan_core.h>



#include "config.h"
#include "ios/logger.h"

namespace vulkan_common {

	/*
	 * INITIALIZATION
	 */
	
	void vulkan_init()
	{
		create_instance();
		create_validation_layers();
		logger::validate("initialized vulkan");
	}
	
	VkDebugUtilsMessengerEXT debugMessenger;
		
	void create_instance()
	{
		/* Set application information */
		VkApplicationInfo vkAppInfo{};
		vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		vkAppInfo.pApplicationName = config::application_name;
		vkAppInfo.applicationVersion = VK_MAKE_VERSION(config::application_version_major, config::application_version_minor, config::application_version_patch);
		vkAppInfo.pEngineName = config::engine_name;
		vkAppInfo.engineVersion = VK_MAKE_VERSION(config::engine_version_major, config::engine_version_minor, config::engine_version_patch);
		vkAppInfo.apiVersion = VK_API_VERSION_1_2;

		/* Initialize vulkan instance */
		VkInstanceCreateInfo vkInstanceCreateInfo{};
		vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		vkInstanceCreateInfo.pApplicationInfo = &vkAppInfo;

		/* Select required extensions */
		std::vector<const char*> RequiredExtensions = vulkan_utils::get_required_extensions();
		vkInstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(RequiredExtensions.size());;
		vkInstanceCreateInfo.ppEnabledExtensionNames = RequiredExtensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfos{};

		/* Enable validation layer (optional) */
		if (config::use_validation_layers)
		{
			vkInstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(config::required_validation_layers.size());
			vkInstanceCreateInfo.ppEnabledLayerNames = config::required_validation_layers.data();
			logger::log("Linked validation layers");
			debugMessengerCreateInfos = static_cast<VkDebugUtilsMessengerCreateInfoEXT>(vulkan_utils::debug_messenger_create_infos);
			vkInstanceCreateInfo.pNext = &debugMessengerCreateInfos;
		}
		else
		{
			vkInstanceCreateInfo.enabledLayerCount = 0;
			vkInstanceCreateInfo.pNext = nullptr;
		}
		
		VK_ENSURE(vkCreateInstance(&vkInstanceCreateInfo, allocation_callback, &instance), "Failed to create vulkan instance");
		logger::log("Created vulkan instance");
		VK_CHECK(instance, "VkInstance is null");
	}

	void create_validation_layers()
	{
		if (!config::use_validation_layers) return;


		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if (func != nullptr) {
			if (func(instance, &vulkan_utils::debug_messenger_create_infos, nullptr, &debugMessenger) != VK_SUCCESS)
			{
				logger::fail("Failed to create debug messenger");
			}
		}
		else {
			logger::fail("Cannot create debug messenger : cannot find required extension");
		}
		logger::log("enabled validation layers");		
	}

	/*
	 * PER WINDOW INSTANCE
	 */


	void close()
	{
		
	}
}
