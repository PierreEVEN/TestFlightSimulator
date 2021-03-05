
#include "assets/shader.h"

#include <fstream>
#include <optional>


#include "engine/jobSystem/job_system.h"
#include "rendering/window.h"
#include "ui/window/windows/profiler.h"


Shader::Shader(const std::filesystem::path& vertex_shader_path,
               const std::filesystem::path& fragment_shader_path, const std::filesystem::path& geometry_shader_path)
{
	shader_creation_vertex = job_system::new_job([&, vertex_shader_path, fragment_shader_path, geometry_shader_path]
		{
			BEGIN_NAMED_RECORD(CREATE_SHADER);
			vertex_module = (std::make_shared<ShaderModule>(window_context->get_context()->logical_device, vertex_shader_path, shaderc_vertex_shader));

		});
	shader_creation_fragment = job_system::new_job([&, vertex_shader_path, fragment_shader_path, geometry_shader_path]
		{
			BEGIN_NAMED_RECORD(CREATE_SHADER);
			fragment_module = (std::make_shared<ShaderModule>(window_context->get_context()->logical_device, fragment_shader_path, shaderc_fragment_shader));

		});
	shader_creation_geometry = job_system::new_job([&, vertex_shader_path, fragment_shader_path, geometry_shader_path]
		{
			BEGIN_NAMED_RECORD(CREATE_SHADER);
			geometry_module = (std::make_shared<ShaderModule>(window_context->get_context()->logical_device, geometry_shader_path, shaderc_geometry_shader));

		});
	logger_log("loaded shader %s", asset_id->to_string().c_str());
}

Shader::~Shader()
{
	shader_creation_vertex->wait();
	shader_creation_fragment->wait();
	shader_creation_geometry->wait();
}
