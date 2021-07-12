#pragma once
#include "asset_base.h"
#include "spirv_cross.hpp"

#include <vulkan/vulkan_core.h>

enum class EShaderStage
{
    VertexShader,
    FragmentShader,
    GeometryShader
};

enum class EShaderPropertyType
{
    Scalar,
    Sampler,
    Structure,
};

struct ShaderProperty
{
    std::string                     property_name;
    spirv_cross::SPIRType::BaseType property_type;
    size_t                          structure_size;
    uint32_t                        location;
    uint32_t                        vec_size;
    EShaderStage                    shader_stage;

    std::vector<ShaderProperty> structure_properties;
};

class Shader : public AssetBase
{
  public:
    Shader(const std::filesystem::path& source_mesh_path, EShaderStage in_shader_kind);
    virtual ~Shader() override;
    [[nodiscard]] VkShaderModule get_shader_module() const
    {
        return shader_module;
    }

    [[nodiscard]] const std::optional<ShaderProperty>& get_push_constants() const
    {
        return push_constants;
    }

    [[nodiscard]] const std::vector<ShaderProperty>& get_uniform_buffers() const
    {
        return uniform_buffer;
    }

    [[nodiscard]] const std::vector<ShaderProperty>& get_sampled_images() const
    {
        return sampled_images;
    }

    [[nodiscard]] const std::vector<ShaderProperty>& get_storage_buffers() const
    {
        return storage_buffer;
    }

  private:
    void                                 build_reflection_data(const std::vector<uint32_t>& bytecode);
    std::optional<std::string>           read_shader_file(const std::filesystem::path& source_path);
    std::optional<std::vector<uint32_t>> compile_module(const std::string& file_name, const std::string& shader_code, EShaderStage shader_kind, bool optimize = true);
    std::optional<VkShaderModule>        create_shader_module(VkDevice logical_device, const std::vector<uint32_t>& bytecode);

    const EShaderStage shader_stage;
    VkShaderModule     shader_module = VK_NULL_HANDLE;

    // Reflection data
    std::string                   entry_point;
    std::optional<ShaderProperty> push_constants = std::optional<ShaderProperty>();
    std::vector<ShaderProperty>   uniform_buffer = {};
    std::vector<ShaderProperty>   storage_buffer = {};
    std::vector<ShaderProperty>   sampled_images = {};
};