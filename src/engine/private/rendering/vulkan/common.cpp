#include "rendering/vulkan/common.h"



#include <set>
#include <vector>
#include <vulkan/vulkan_core.h>



#include "config.h"

#include <cpputils/logger.hpp>

namespace vulkan_common {

	/*
	 * INITIALIZATION
	 */
	
	void vulkan_init()
	{
		create_instance();
		create_validation_layers();
                LOG_VALIDATE("initialized vulkan");
	}

	void vulkan_shutdown()
	{
		destroy_validation_layers();
		destroy_instance();
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
                        LOG_INFO("Linked validation layers");
			debugMessengerCreateInfos = static_cast<VkDebugUtilsMessengerCreateInfoEXT>(vulkan_utils::debug_messenger_create_infos);
			vkInstanceCreateInfo.pNext = &debugMessengerCreateInfos;
		}
		else
		{
			vkInstanceCreateInfo.enabledLayerCount = 0;
			vkInstanceCreateInfo.pNext = nullptr;
		}
		
		VK_ENSURE(vkCreateInstance(&vkInstanceCreateInfo, allocation_callback, &instance), "Failed to create vulkan instance");
                LOG_INFO("Created vulkan instance");
		VK_CHECK(instance, "VkInstance is null");
	}

	void create_validation_layers()
	{
		if (!config::use_validation_layers) return;


		auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if (func != nullptr) {
			if (func(instance, &vulkan_utils::debug_messenger_create_infos, nullptr, &debugMessenger) != VK_SUCCESS)
			{ LOG_FATAL("Failed to create debug messenger");
			}
		}
		else {
                    LOG_FATAL("Cannot create debug messenger : cannot find required extension");
		}
                LOG_INFO("enabled validation layers");
	}

	void destroy_validation_layers()
	{
		if (!config::use_validation_layers) return;

		LOG_INFO("Destroy validation layers");
		auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (func) func(instance, debugMessenger, allocation_callback);
	}

	void destroy_instance()
	{
            LOG_INFO("Destroy Vulkan instance");
		vkDestroyInstance(instance, allocation_callback);
	}
}
