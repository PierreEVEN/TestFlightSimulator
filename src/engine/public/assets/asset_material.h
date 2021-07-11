#pragma once
#include "asset_base.h"
#include "asset_mesh_data.h"
#include "asset_shader.h"
#include "rendering/vulkan/descriptor_pool.h"

class UniformBuffer;
class Shader;

enum class EShaderParameterUsage
{
    PushConstant,
    UniformBuffer
};

class ShaderParameter
{
    friend class Material;

  public:
    template <typename Parameter_T, typename... Args_T> static std::shared_ptr<ShaderParameter> create(const std::string& name, EShaderStage stage, Args_T&&... arguments)
    {
        std::shared_ptr<ShaderParameter> parameter = std::make_shared<Parameter_T>(std::forward<Args_T>(arguments)...);
        parameter->parameter_name                  = name;
        parameter->shader_stage                    = stage;
        parameter->mark_dirty();
        return parameter;
    }

    void mark_dirty()
    {
        for (int i = 0; i < dirty_buffers.size(); ++i)
        {
            dirty_buffers[i] = true;
        }
    }

    virtual VkWriteDescriptorSet generate_descriptor_sets() = 0;

  protected:
    ShaderParameter() = default;

  private:
    std::string           parameter_name;
    EShaderStage          shader_stage;
    std::vector<bool>     dirty_buffers;
    EShaderParameterUsage buffer_usage;
};

class PushConstant
{
  public:
    template <typename Struct_T> PushConstant(const Struct_T& in_data)
    {
        set_data(in_data);
    }

    template <typename Struct_T> void set_data(const Struct_T& in_data)
    {
        data_size = sizeof(Struct_T);
        data      = realloc(data, data_size);
        if (data)
        {
            memcpy(data, &in_data, data_size);
            b_is_dirty = true;
        }
        else
        {
            LOG_ERROR("failed to realloc push_constant memory");
        }
    }

    [[nodiscard]] size_t get_size() const
    {
        return data_size;
    }
    [[nodiscard]] void* get_data() const
    {
        if (!data)
            LOG_ERROR("trying to retrieve push constant data from uninitialized push constants. size = %d", data_size);
        return data;
    }

    VkShaderStageFlags stage_flags;

  private:
    size_t data_size  = 0;
    void*  data       = nullptr;
    bool   b_is_dirty = false;
};

struct ShaderStageData
{
    TAssetPtr<Shader>                     shader         = nullptr;
    std::vector<TAssetPtr<UniformBuffer>> uniform_buffer = {};
};

class Material : public AssetBase
{
  public:
    Material(const ShaderStageData& in_vertex_stage, const ShaderStageData& in_fragment_stage, const std::shared_ptr<PushConstant>& in_push_constant = nullptr);
    virtual ~Material() override;

    [[nodiscard]] VkPipelineLayout get_pipeline_layout() const
    {
        return pipeline_layout;
    }
    [[nodiscard]] VkPipeline get_pipeline() const
    {
        return shader_pipeline;
    }
    [[nodiscard]] const std::vector<VkDescriptorSet>& get_descriptor_sets() const
    {
        return descriptor_sets;
    }
    void update_push_constants(VkCommandBuffer& command_buffer);

    void update_descriptor_sets(size_t imageIndex);
    
    void set_push_constant_model_matrix(const glm::mat4& mat) const
    {
        push_constant->set_data(mat);
    }

  private:
    void destroy_resources();

    std::vector<VkDescriptorSetLayoutBinding> make_layout_bindings();
    void                                      create_pipeline();
    void                                      create_descriptor_sets(std::vector<VkDescriptorSetLayoutBinding> layoutBindings);

    ShaderStageData               vertex_stage   = {};
    ShaderStageData               fragment_stage = {};
    std::shared_ptr<PushConstant> push_constant  = nullptr;
    
    std::unordered_map<std::string, uint32_t> vertex_uniform_bindings;
    std::unordered_map<std::string, uint32_t> fragment_uniform_bindings;

    VkDescriptorSetLayout        descriptor_set_layout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptor_sets       = {};
    VkPipelineLayout             pipeline_layout       = VK_NULL_HANDLE;
    VkPipeline                   shader_pipeline       = VK_NULL_HANDLE;
};