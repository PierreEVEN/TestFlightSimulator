#pragma once
#include <filesystem>
#include <optional>

#include "GraphicResource.h"
#include "rendering/vulkan/shaderModule.h"

class Window;

enum class EShaderStage
{
	SS_VERTEX,
	SS_FRAGMENT,
	SS_GEOMETRY
};

class Shader : public GraphicResource
{
protected:
	friend GraphicResource;
	
	Shader(Window* context, const AssetRef& asset_ref, const std::filesystem::path& vertex_shader_path = "", const std::filesystem::path fragment_shader_path = "", const std::filesystem::path geometry_shader_path = "");

public:
private:
	std::shared_ptr<ShaderModule> vertex_module;
	std::shared_ptr<ShaderModule> fragment_module;
	std::shared_ptr<ShaderModule> geometry_module;
};
