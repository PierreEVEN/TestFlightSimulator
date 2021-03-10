#pragma once
#include <filesystem>
#include <optional>


#include "assetBase.h"
#include "GraphicResource.h"
#include "rendering/vulkan/shaderModule.h"

#include "jobSystem/job.h"

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
		return shader_creation_job->is_complete;
	}

	[[nodiscard]] VkDescriptorSet& get_descriptor_set(uint8_t image_index) { return descriptor_sets[image_index]; }
	[[nodiscard]] VkPipeline get_pipeline() const { return shader_pipeline; }
	[[nodiscard]] VkPipelineLayout get_pipeline_layout() const { return pipeline_layout; }


protected:
	friend AssetManager;
	Shader(const std::filesystem::path& vertex_shader_path = "", const std::filesystem::path& fragment_shader_path = "", const std::filesystem::path& geometry_shader_path = "");
	~Shader();
private:

	void create_pipeline();
	void create_descriptor_sets(std::vector<VkDescriptorSetLayoutBinding> layout_bindings);
	std::vector<VkDescriptorSetLayoutBinding> create_layout_bindings();
	void destroy();


	std::vector<std::shared_ptr<UniformBase>> uniforms;

	std::shared_ptr<ShaderModule> vertex_module;
	std::shared_ptr<ShaderModule> fragment_module;
	std::shared_ptr<ShaderModule> geometry_module;

	VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> descriptor_sets;
	VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
	VkPipeline shader_pipeline = VK_NULL_HANDLE;

	std::shared_ptr<job_system::IJobTask> shader_creation_job;
};
