#pragma once
#include <string>

#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan_core.h>

namespace std {
	namespace filesystem {
		class path;
	}
}

class ShaderModule final
{
public:
	ShaderModule(VkDevice logical_device, std::string file_name, const std::string& shader_code, shaderc_shader_kind shader_kind);
	ShaderModule(VkDevice logical_device, std::filesystem::path source_path, shaderc_shader_kind shader_kind);

	~ShaderModule();
private:
	const VkDevice device;
	VkShaderModule shader_module = VK_NULL_HANDLE;
};