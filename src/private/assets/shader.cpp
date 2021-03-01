
#include "assets/shader.h"

#include <fstream>
#include <optional>

#include "rendering/window.h"


Shader::Shader(Window* context, const AssetRef& asset_ref, const std::filesystem::path& vertex_shader_path,
	const std::filesystem::path fragment_shader_path, const std::filesystem::path geometry_shader_path)
	:
	vertex_module(std::make_shared<ShaderModule>(context->get_context()->logical_device, vertex_shader_path, shaderc_vertex_shader)),
	fragment_module(std::make_shared<ShaderModule>(context->get_context()->logical_device, fragment_shader_path, shaderc_fragment_shader)),
	geometry_module(std::make_shared<ShaderModule>(context->get_context()->logical_device, geometry_shader_path, shaderc_geometry_shader)),
	GraphicResource(context, asset_ref)
{
	logger_log("create shader %s", asset_ref.to_string().c_str());
}
