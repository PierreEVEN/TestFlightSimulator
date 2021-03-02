#pragma once
#include <filesystem>
#include <optional>


#include "assetBase.h"
#include "GraphicResource.h"
#include "rendering/vulkan/shaderModule.h"

#include "engine/jobSystem/job.h"

class Window;

enum class EShaderStage
{
	SS_VERTEX,
	SS_FRAGMENT,
	SS_GEOMETRY
};

class Shader : public AssetBase
{
protected:
	friend AssetManager;
	Shader(const std::filesystem::path& vertex_shader_path = "", const std::filesystem::path& fragment_shader_path = "", const std::filesystem::path& geometry_shader_path = "");
	~Shader();
private:
	std::shared_ptr<ShaderModule> vertex_module;
	std::shared_ptr<ShaderModule> fragment_module;
	std::shared_ptr<ShaderModule> geometry_module;

	std::shared_ptr<job_system::IJobTask> shader_creation_task;
};
