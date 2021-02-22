#include "misc/capabilities.h"


#include <string>
#include <vector>


#include "config.h"
#include "ios/logger.h"
#include "rendering/Window.h"

namespace capabilities
{
	void check_all()
	{
		logger::log("checking vulkan support");
		check_vk_extensions();
		if (config::use_validation_layers) check_validation_layer_support();
		logger::validate("vulkan is fully supported !");
	}

	void check_vk_extensions()
	{
		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
		std::vector<VkExtensionProperties> extensions(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

		std::string log = "detected " + std::to_string(extension_count) + " supported extensions\n";

		for (const VkExtensionProperties& extension : extensions) {
			log += "\t-" + std::string(extension.extensionName) + "\n";
		}

		std::vector<const char*> required_extensions = vulkan_utils::get_required_extensions();

		logger::log(log.c_str());

		for (const auto& required_ext : required_extensions)
		{
			bool found_extension = false;
			for (const auto& ext : extensions)
			{
				if (!strcmp(ext.extensionName, required_ext))
				{
					found_extension = true;
					break;
				}
			}
			if (!found_extension) logger::error("extension '%s' is required but cannot be found.", required_ext);
		}
		logger::log("all required extensions are supported !");
	}

	void check_validation_layer_support()
	{
		uint32_t layer_count;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		std::vector<VkLayerProperties> available_layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

		for (const char* layer_name : config::required_validation_layers) {
			bool found_layer = false;

			for (const auto& layer_properties : available_layers) {
				if (strcmp(layer_name, layer_properties.layerName) == 0) {
					found_layer = true;
					break;
				}
			}

			if (!found_layer) logger::warning("failed to enable required validation layer '%s'", layer_name);
		}
		logger::log("validation layers are supported");
	}
}
