#pragma once
#include <filesystem>
#include <optional>


#include "assetBase.h"
#include "GraphicResource.h"
#include "rendering/vulkan/shaderModule.h"

#include "engine/jobSystem/job.h"

class UniformBase;
class Window;

enum class EShaderStage
{
	SS_VERTEX,
	SS_FRAGMENT,
	SS_GEOMETRY
};

class Shader : public AssetBase
{
public:
	bool try_load() override
	{
		return shader_creation_vertex->is_complete && shader_creation_fragment->is_complete && shader_creation_geometry->is_complete;
	}
protected:
	friend AssetManager;
	Shader(const std::filesystem::path& vertex_shader_path = "", const std::filesystem::path& fragment_shader_path = "", const std::filesystem::path& geometry_shader_path = "");
	~Shader();
private:

	std::vector<std::shared_ptr<UniformBase>> uniforms;
	
	std::shared_ptr<ShaderModule> vertex_module;
	std::shared_ptr<ShaderModule> fragment_module;
	std::shared_ptr<ShaderModule> geometry_module;

	std::shared_ptr<job_system::IJobTask> shader_creation_vertex;
	std::shared_ptr<job_system::IJobTask> shader_creation_fragment;
	std::shared_ptr<job_system::IJobTask> shader_creation_geometry;
};
