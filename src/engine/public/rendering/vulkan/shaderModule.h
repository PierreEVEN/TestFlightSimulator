#pragma once
#include <spirv_cross.hpp>
#include <string>

//#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan_core.h>
#include "glslang/Include/glslang_c_shader_types.h"

namespace std {
	namespace filesystem {
		class path;
	}
}

struct descriptor_set
{
	VkDescriptorType type;
};


class ShaderModule final
{
public:
    ShaderModule(VkDevice logical_device, std::string in_file_name, const std::string& shader_code, glslang_stage_t shader_kind);
  ShaderModule(VkDevice logical_device, std::filesystem::path source_path, glslang_stage_t shader_kind);

	~ShaderModule();

	[[nodiscard]] VkShaderModule get() const { return shader_module; }
	[[nodiscard]] std::string get_file_name() const { return file_name; }
	[[nodiscard]] uint32_t get_push_constant_size() const { return push_constant_buffer_size; }
	[[nodiscard]] int32_t get_uniform_bindings() const { return uniform_buffer_binding; }
	[[nodiscard]] std::vector<uint32_t> get_sampled_image_bindings() const { return sampled_image_bindings; }

private:

	void build_reflection_data(const std::vector<uint32_t>& bytecode);
	
	const std::string file_name;
	const VkDevice device;
	VkShaderModule shader_module = VK_NULL_HANDLE;

	/* Reflection data */
	
	uint32_t push_constant_buffer_size = 0;
	int32_t uniform_buffer_binding = -1;
	std::vector<uint32_t> sampled_image_bindings;
};