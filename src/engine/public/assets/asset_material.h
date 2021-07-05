#pragma once
#include "asset_base.h"
#include "asset_mesh_data.h"
#include "rendering/vulkan/descriptor_pool.h"

namespace glTF2
{
struct Texture;
}

class Shader;

enum class EShaderParameterType
{
    Integer,
    Float,
    TextureSampler,
    Undefined
};

class ShaderParameter
{
  public:
    virtual VkWriteDescriptorSet get_descriptor_set() const = 0;
};

class FloatShaderParameter : public ShaderParameter
{

  private:
    float value;
};

class IntShaderParameter : public ShaderParameter
{

  private:
    int32_t value;
};

class Dmat4ShaderParameter : public ShaderParameter
{

  private:
    glm::dmat4 value;
};

class TextureShaderParameter : public ShaderParameter
{
  public:
    VkWriteDescriptorSet get_descriptor_set() const override;

  private:
    TAssetPtr<glTF2::Texture> value;
};

class Material : public AssetBase
{
  public:
    Material(const TAssetPtr<Shader>& in_vertex_shader, const TAssetPtr<Shader>& in_fragment_shader);
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

  private:
    typedef int EMaterialCreationFlags;
    enum EMaterialCreationFlags_
    {
        EMATERIAL_CREATION_FLAG_NONE                = 0,
        EMATERIAL_CREATION_FLAG_TRANSLUCENT         = 1 << 0,
        EMATERIAL_CREATION_FLAG_WIREFRAME           = 1 << 1,
        EMATERIAL_CREATION_FLAG_DOUBLESIDED         = 1 << 2,
        EMATERIAL_CREATION_FLAG_DISABLE_DEPTH_TEST  = 1 << 3,
        EMATERIAL_CREATION_FLAG_ENABLE_STENCIL_TEST = 1 << 4,
    };
    struct SMaterialStaticProperties
    {
        bool                        bUseGlobalUbo          = true;
        uint32_t                    VertexTexture2DCount   = 0;
        uint32_t                    FragmentTexture2DCount = 0;
        EMaterialCreationFlags      materialCreationFlag   = EMATERIAL_CREATION_FLAG_NONE;
        class VertexShaderModule*   vertexShaderModule     = nullptr;
        class FragmentShaderModule* fragmentShaderModule   = nullptr;

        inline bool operator==(const SMaterialStaticProperties& other) const
        {
            return false;
        }
    };

    void destroy_resources();

    static std::vector<VkDescriptorSetLayoutBinding> MakeLayoutBindings(const SMaterialStaticProperties& materialProperties);
    void                                             create_pipeline(const SMaterialStaticProperties& materialProperties);
    void                                             create_descriptor_sets(std::vector<VkDescriptorSetLayoutBinding> layoutBindings);

    std::vector<ShaderParameter> vertex_shader_parameters   = {};
    TAssetPtr<Shader>            vertex_shader              = nullptr;
    std::vector<ShaderParameter> fragment_shader_parameters = {};
    TAssetPtr<Shader>            fragment_shader            = nullptr;

    VkDescriptorSetLayout        descriptorSetLayout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptor_sets      = {};
    VkPipelineLayout             pipeline_layout      = VK_NULL_HANDLE;
    VkPipeline                   shader_pipeline      = VK_NULL_HANDLE;
};