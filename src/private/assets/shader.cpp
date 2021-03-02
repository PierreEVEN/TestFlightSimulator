
#include "assets/shader.h"

#include <fstream>
#include <optional>


#include "engine/jobSystem/job_system.h"
#include "rendering/window.h"


Shader::Shader(const std::filesystem::path& vertex_shader_path,
	const std::filesystem::path& fragment_shader_path, const std::filesystem::path& geometry_shader_path)
{
	shader_creation_task = job_system::new_job([&, vertex_shader_path, fragment_shader_path, geometry_shader_path]
		{
			vertex_module = (std::make_shared<ShaderModule>(window_context->get_context()->logical_device, vertex_shader_path, shaderc_vertex_shader));
			fragment_module = (std::make_shared<ShaderModule>(window_context->get_context()->logical_device, fragment_shader_path, shaderc_fragment_shader));
			geometry_module = (std::make_shared<ShaderModule>(window_context->get_context()->logical_device, geometry_shader_path, shaderc_geometry_shader));

			logger_log("loaded shader %s", asset_id->to_string().c_str());
		});
}

Shader::~Shader()
{
	shader_creation_task->wait();
}
