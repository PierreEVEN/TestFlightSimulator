
#include "rendering/vulkan/shaderModule.h"

#include <optional>
#include <string>
#include <fstream>
#include <vulkan/vulkan_core.h>



#include <cpputils/logger.hpp>

#include "spirv_glsl.hpp"
#include "rendering/vulkan/common.h"
#include "ui/window/windows/profiler.h"

#define ENABLE_SHADER_LOGGING true


std::optional<std::string> read_shader_file(const std::filesystem::path& source_path)
{
	std::optional<std::string> code;

	if (exists(source_path)) {
		std::ifstream shader_file(source_path);

		std::string code_string;
		std::string line;
		while (std::getline(shader_file, line)) {
			code_string += line + "\n";
		}

		shader_file.close();
		code = code_string;
	}

	return code;
}

std::optional<std::vector<uint32_t>> compile_module(const std::string& file_name, const std::string& shader_code, shaderc_shader_kind shader_kind, bool optimize = true)
{
	BEGIN_NAMED_RECORD(COMPILE_SHADER_MODULE);
	shaderc::Compiler Compiler;
	shaderc::CompileOptions Options;
	std::optional<std::vector<uint32_t>> bytecode;
	
	switch (shader_kind)
	{
	case shaderc_vertex_shader:
		Options.AddMacroDefinition("VERTEX_SHADER");
		break;
	case shaderc_fragment_shader:
		Options.AddMacroDefinition("FRAGMENT_SHADER");
		break;
	case shaderc_geometry_shader:
		Options.AddMacroDefinition("GEOMETRY_SHADER");
		break;
	default: break;
	}

	const shaderc::PreprocessedSourceCompilationResult preprocess_result = Compiler.PreprocessGlsl(shader_code.data(), shader_kind, file_name.c_str(), Options);

	if (preprocess_result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
            LOG_ERROR("failed to prprocess shader %s : %s", file_name.c_str(), preprocess_result.GetErrorMessage().c_str());
		return bytecode;
	}
	std::vector<char> preprocessed_shader = { preprocess_result.cbegin(), preprocess_result.cend() };

	Options.SetTargetEnvironment(shaderc_target_env_vulkan, 0);

#if _DEBUG
	Options.SetOptimizationLevel(optimize ?
		shaderc_optimization_level_size : shaderc_optimization_level_zero);
#else
	Options.SetOptimizationLevel(optimize ?
		shaderc_optimization_level_performance : shaderc_optimization_level_zero);
#endif

	const shaderc::SpvCompilationResult compilation_result = Compiler.CompileGlslToSpv(static_cast<char*>(preprocessed_shader.data()), preprocessed_shader.size(), shader_kind, file_name.c_str(), "main", Options);

	if (compilation_result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
            LOG_ERROR("Failed to compile shader %s : %s", file_name.c_str(), compilation_result.GetErrorMessage().c_str());
		return bytecode;
	}
	bytecode = { compilation_result.cbegin(), compilation_result.cend() };

	return bytecode;
}

std::optional<VkShaderModule> create_shader_module(VkDevice logical_device, const std::vector<uint32_t>& bytecode)
{
	VkShaderModule shader_module;
	
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = bytecode.size() * sizeof(uint32_t);
	createInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());
	VkResult res = vkCreateShaderModule(logical_device, &createInfo, vulkan_common::allocation_callback, &shader_module);
	if (!res == VK_SUCCESS) {
            LOG_ERROR("Failed to create shader module : %d", static_cast<uint32_t>(res));
		return nullptr;
	}
	return shader_module;
}

ShaderModule::ShaderModule(VkDevice logical_device, std::string in_file_name, const std::string& shader_code,
	shaderc_shader_kind shader_kind)
	: device(logical_device), file_name(in_file_name)
{
	if (auto bytecode = compile_module(file_name, shader_code, shader_kind))
	{
		if (auto shader_m = create_shader_module(logical_device, bytecode.value()))
		{
			build_reflection_data(bytecode.value());
			shader_module = shader_m.value();
		}
	}
}

ShaderModule::ShaderModule(VkDevice logical_device, std::filesystem::path source_path, shaderc_shader_kind shader_kind)
	: device(logical_device), file_name(source_path.string())
{
	auto shader_data = read_shader_file(source_path);
	if (shader_data) {
		if (auto bytecode = compile_module(source_path.string(), shader_data.value(), shader_kind))
		{
			if (auto shader_m = create_shader_module(logical_device, bytecode.value()))
			{
				build_reflection_data(bytecode.value());
				shader_module = shader_m.value();
			}
		}
	}
}

ShaderModule::~ShaderModule()
{
	vkDestroyShaderModule(device, shader_module, vulkan_common::allocation_callback);
}

void ShaderModule::build_reflection_data(const std::vector<uint32_t>& bytecode)
{
	const spirv_cross::Compiler compiler(bytecode);

#if ENABLE_SHADER_LOGGING
        LOG_INFO("### BUILDING SHADER ###");
#endif

	/**
	 *  PUSH CONSTANTS
	 */


	const auto push_constant_buffers = compiler.get_shader_resources().push_constant_buffers;
	
#if ENABLE_SHADER_LOGGING
        LOG_INFO("push constant : %d", push_constant_buffers.size());
	for (auto& push_constant : push_constant_buffers)
	{ LOG_INFO("\t=> #%d (%s)", push_constant.id, push_constant.name.c_str());
	}
#endif

	push_constant_buffer_size = 0;
	if (!push_constant_buffers.empty())
	{
            if (push_constant_buffers.size() != 1) LOG_ERROR("unhandled push constant buffer count");
		
		for (auto& buffer_range : compiler.get_active_buffer_ranges(compiler.get_shader_resources().push_constant_buffers[0].id)) {
#if ENABLE_SHADER_LOGGING
                    LOG_INFO("\t\t => Accessing member # % u, offset % u, size % u", buffer_range.index, buffer_range.offset, buffer_range.range);
#endif
			push_constant_buffer_size += static_cast<uint32_t>(buffer_range.range);
		}
		
#if ENABLE_SHADER_LOGGING
                LOG_INFO("push constant size : %d", push_constant_buffer_size);
#endif
	}


	/**
	 *  UNIFORM BUFFERS
	 */

	const auto uniform_buffers = compiler.get_shader_resources().uniform_buffers;
	
#if ENABLE_SHADER_LOGGING
        LOG_INFO("uniform buffers : %d", uniform_buffers.size());
#endif
	
	if (!uniform_buffers.empty())
	{
            if (uniform_buffers.size() != 1) LOG_ERROR("unhandled uniform buffer count");
		for (auto& buffer : uniform_buffers)
		{
			uint32_t set = compiler.get_decoration(buffer.id, spv::DecorationDescriptorSet);
			uniform_buffer_binding = compiler.get_decoration(buffer.id, spv::DecorationBinding);
#if ENABLE_SHADER_LOGGING
                        LOG_INFO("\t => Found UBO % s at set = %u, binding = %u!", buffer.name.c_str(), set, uniform_buffer_binding);
#endif
		}
	}

	/**
	 *  IMAGE SAMPLERS
	 */

	const auto sampled_image = compiler.get_shader_resources().sampled_images;
	
#if ENABLE_SHADER_LOGGING
        LOG_INFO("uniform sampler : %d", sampled_image.size());
#endif
	
	if (!sampled_image.empty())
	{
		for (const auto& sampler : sampled_image)
		{
			uint32_t set = compiler.get_decoration(sampler.id, spv::DecorationDescriptorSet);
			uint32_t sampled_image_binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);
#if ENABLE_SHADER_LOGGING
                        LOG_INFO("\t => Found sampled image % s at set = % u, binding = % u!", sampler.name.c_str(), set, sampled_image_binding);
#endif
			sampled_image_bindings.push_back(sampled_image_binding);
		}
	}
}