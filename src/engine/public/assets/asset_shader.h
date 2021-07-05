#pragma once
#include "asset_base.h"

#include <vulkan/vulkan_core.h>

enum class EShaderKind
{
    VertexShader,
    FragmentShader,
    GeometryShader
};

class Shader : public AssetBase
{
  public:
    Shader(const std::filesystem::path& source_mesh_path, EShaderKind in_shader_kind);
    virtual ~Shader() override;
    [[nodiscard]] VkShaderModule get_shader_module() const
    {
        return shader_module;
    }

  private:
    void                                 build_reflection_data(const std::vector<uint32_t>& bytecode);
    std::optional<std::string>           read_shader_file(const std::filesystem::path& source_path);
    std::optional<std::vector<uint32_t>> compile_module(const std::string& file_name, const std::string& shader_code, EShaderKind shader_kind, bool optimize = true);
    std::optional<VkShaderModule>        create_shader_module(VkDevice logical_device, const std::vector<uint32_t>& bytecode);

    EShaderKind    shader_kind;
    VkShaderModule shader_module = VK_NULL_HANDLE;

    /* Reflection data */

    uint32_t              push_constant_buffer_size = 0;
    int32_t               uniform_buffer_binding    = -1;
    std::vector<uint32_t> sampled_image_bindings;
};