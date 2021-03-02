
#include "rendering/vulkan/shaderModule.h"

#include <filesystem>
#include <optional>
#include <string>
#include <fstream>
#include <vulkan/vulkan_core.h>


#include "ios/logger.h"

#include "spirv_glsl.hpp"
#include "rendering/vulkan/common.h"

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
		logger_error("failed to prprocess shader %s : %s", file_name.c_str(), preprocess_result.GetErrorMessage().c_str());
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
		logger_error("Failed to compile shader %s : %s", file_name.c_str(), compilation_result.GetErrorMessage().c_str());
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
		logger_error("Failed to create shader module : %d", static_cast<uint32_t>(res));
		return nullptr;
	}
	return shader_module;
}

ShaderModule::ShaderModule(VkDevice logical_device, std::string file_name, const std::string& shader_code,
	shaderc_shader_kind shader_kind)
	: device(logical_device)
{
	if (auto bytecode = compile_module(file_name, shader_code, shader_kind))
	{
		if (auto shader_m = create_shader_module(logical_device, bytecode.value()))
		{
			shader_module = shader_m.value();
		}
	}
}

ShaderModule::ShaderModule(VkDevice logical_device, std::filesystem::path source_path, shaderc_shader_kind shader_kind)
	: device(logical_device)
{
	auto shader_data = read_shader_file(source_path);
	if (shader_data) {
		if (auto bytecode = compile_module(source_path.string(), shader_data.value(), shader_kind))
		{
			if (auto shader_m = create_shader_module(logical_device, bytecode.value()))
			{
				shader_module = shader_m.value();				
			}
		}
	}
}

ShaderModule::~ShaderModule()
{
	vkDestroyShaderModule(device, shader_module, vulkan_common::allocation_callback);
}
