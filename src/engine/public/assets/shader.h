#pragma once
#include <unordered_map>


#include "assetBase.h"
#include "graphicResource.h"
#include "rendering/vulkan/shaderModule.h"
#include <filesystem>

#include "jobSystem/job.h"

class Texture2d;
class UniformBase;
class Window;

class DescriptorState final
{
public:
	~DescriptorState()
	{
		if (buffer) free(buffer);
	}
	
	bool needs_update = true;
	uint32_t binding;
	std::optional<TAssetPtr<Texture2d>> texture;
	void* buffer = nullptr;
};


class Shader : public AssetBase
{
public:
	bool try_load() override
	{
		if (!shader_creation_job->is_complete()) return false;
		
		return true;
	}

	[[nodiscard]] VkDescriptorSet& get_descriptor_set(uint8_t image_index) { return descriptor_sets[image_index]; }
	[[nodiscard]] VkPipeline get_pipeline() const { return shader_pipeline; }
	[[nodiscard]] VkPipelineLayout get_pipeline_layout() const { return pipeline_layout; }

	template<typename Struct>
	void push_constant_value(VkCommandBuffer command_buffer, Struct* data, VkShaderStageFlags shader_stage)
	{
		auto shader_module_stage = get_shader_module(shader_stage);
            if (!shader_module_stage) LOG_FATAL("%d is not a valid shader stage", shader_stage);
                if (shader_module_stage->get_push_constant_size() != sizeof(Struct)) LOG_FATAL("push constant error : %d is not a valid buffer size (expected %d)", sizeof(Struct), shader_module_stage->get_push_constant_size());
		vkCmdPushConstants(command_buffer, pipeline_layout, shader_stage, 0, sizeof(Struct), data);
	}

	void bind_texture(uint32_t binding, TAssetPtr<Texture2d> texture);
	void bind_buffer(uint32_t binding, void* buffer_data);

protected:
	friend AssetManager;
	Shader(const std::filesystem::path& vertex_shader_path = "", const std::filesystem::path& fragment_shader_path = "", const std::filesystem::path& geometry_shader_path = "");
	~Shader();
private:

	void create_pipeline();
	void create_descriptor_sets();
	void destroy();

	void update_descriptors();
	
	std::shared_ptr<ShaderModule> vertex_module;
	std::shared_ptr<ShaderModule> fragment_module;
	std::shared_ptr<ShaderModule> geometry_module;

	std::shared_ptr<ShaderModule> get_shader_module(VkShaderStageFlags shader_stage);
	
	VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
	VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
	VkPipeline shader_pipeline = VK_NULL_HANDLE;

	/** Per frame descriptor states */
	std::vector<std::vector<DescriptorState>> descriptors;
	std::vector<VkDescriptorSet> descriptor_sets;

	std::shared_ptr<job_system::IJobTask> shader_creation_job;
};