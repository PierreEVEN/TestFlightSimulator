

#include "assets/asset_shader.h"

#include "StandAlone/ResourceLimits.h"
#include "glslang/Include/glslang_c_interface.h"
#include "glslang/Include/glslang_c_shader_types.h"
#include "rendering/vulkan/common.h"
#include "spirv_cross.hpp"
#include "statsRecorder.h"
#include "testGameInterface.h"
#include <magic_enum/magic_enum.h>

#define ENABLE_SHADER_LOGGING true

std::optional<std::string> Shader::read_shader_file(const std::filesystem::path& source_path)
{
    std::optional<std::string> code;

    if (exists(source_path))
    {
        std::ifstream shader_file(source_path);

        std::string code_string;
        std::string line;
        while (std::getline(shader_file, line))
        {
            code_string += line + "\n";
        }

        shader_file.close();
        code = code_string;
    }

    return code;
}

Shader::Shader(const std::filesystem::path& source_mesh_path, EShaderStage in_shader_kind) : shader_stage(in_shader_kind)
{
    auto shader_data = read_shader_file(source_mesh_path);
    if (shader_data)
    {
        if (auto bytecode = compile_module(source_mesh_path.string(), shader_data.value(), shader_stage))
        {
            if (auto shader_m = create_shader_module(GfxContext::get()->logical_device, bytecode.value()))
            {
                build_reflection_data(bytecode.value());
                shader_module = shader_m.value();
            }
            else
            {
                LOG_ERROR("failed to create shader module %s", source_mesh_path.string().c_str());
            }
        }
        else
        {
            LOG_ERROR("failed to compile shader file %s", source_mesh_path.string().c_str());
        }
    }
    else
    {
        LOG_ERROR("failed to read shader file %s", source_mesh_path.string().c_str());
    }
}

Shader::~Shader()
{
    vkDestroyShaderModule(GfxContext::get()->logical_device, shader_module, vulkan_common::allocation_callback);
}

static TBuiltInResource glslang_default_resource = {

    .maxLights                                 = 32,
    .maxClipPlanes                             = 6,
    .maxTextureUnits                           = 32,
    .maxTextureCoords                          = 32,
    .maxVertexAttribs                          = 64,
    .maxVertexUniformComponents                = 4096,
    .maxVaryingFloats                          = 64,
    .maxVertexTextureImageUnits                = 32,
    .maxCombinedTextureImageUnits              = 80,
    .maxTextureImageUnits                      = 32,
    .maxFragmentUniformComponents              = 4096,
    .maxDrawBuffers                            = 32,
    .maxVertexUniformVectors                   = 128,
    .maxVaryingVectors                         = 8,
    .maxFragmentUniformVectors                 = 16,
    .maxVertexOutputVectors                    = 16,
    .maxFragmentInputVectors                   = 15,
    .minProgramTexelOffset                     = -8,
    .maxProgramTexelOffset                     = 7,
    .maxClipDistances                          = 8,
    .maxComputeWorkGroupCountX                 = 65535,
    .maxComputeWorkGroupCountY                 = 65535,
    .maxComputeWorkGroupCountZ                 = 65535,
    .maxComputeWorkGroupSizeX                  = 1024,
    .maxComputeWorkGroupSizeY                  = 1024,
    .maxComputeWorkGroupSizeZ                  = 64,
    .maxComputeUniformComponents               = 1024,
    .maxComputeTextureImageUnits               = 16,
    .maxComputeImageUniforms                   = 8,
    .maxComputeAtomicCounters                  = 8,
    .maxComputeAtomicCounterBuffers            = 1,
    .maxVaryingComponents                      = 60,
    .maxVertexOutputComponents                 = 64,
    .maxGeometryInputComponents                = 64,
    .maxGeometryOutputComponents               = 128,
    .maxFragmentInputComponents                = 128,
    .maxImageUnits                             = 8,
    .maxCombinedImageUnitsAndFragmentOutputs   = 8,
    .maxCombinedShaderOutputResources          = 8,
    .maxImageSamples                           = 0,
    .maxVertexImageUniforms                    = 0,
    .maxTessControlImageUniforms               = 0,
    .maxTessEvaluationImageUniforms            = 0,
    .maxGeometryImageUniforms                  = 0,
    .maxFragmentImageUniforms                  = 8,
    .maxCombinedImageUniforms                  = 8,
    .maxGeometryTextureImageUnits              = 16,
    .maxGeometryOutputVertices                 = 256,
    .maxGeometryTotalOutputComponents          = 1024,
    .maxGeometryUniformComponents              = 1024,
    .maxGeometryVaryingComponents              = 64,
    .maxTessControlInputComponents             = 128,
    .maxTessControlOutputComponents            = 128,
    .maxTessControlTextureImageUnits           = 16,
    .maxTessControlUniformComponents           = 1024,
    .maxTessControlTotalOutputComponents       = 4096,
    .maxTessEvaluationInputComponents          = 128,
    .maxTessEvaluationOutputComponents         = 128,
    .maxTessEvaluationTextureImageUnits        = 16,
    .maxTessEvaluationUniformComponents        = 1024,
    .maxTessPatchComponents                    = 120,
    .maxPatchVertices                          = 32,
    .maxTessGenLevel                           = 64,
    .maxViewports                              = 16,
    .maxVertexAtomicCounters                   = 0,
    .maxTessControlAtomicCounters              = 0,
    .maxTessEvaluationAtomicCounters           = 0,
    .maxGeometryAtomicCounters                 = 0,
    .maxFragmentAtomicCounters                 = 8,
    .maxCombinedAtomicCounters                 = 8,
    .maxAtomicCounterBindings                  = 1,
    .maxVertexAtomicCounterBuffers             = 0,
    .maxTessControlAtomicCounterBuffers        = 0,
    .maxTessEvaluationAtomicCounterBuffers     = 0,
    .maxGeometryAtomicCounterBuffers           = 0,
    .maxFragmentAtomicCounterBuffers           = 1,
    .maxCombinedAtomicCounterBuffers           = 1,
    .maxAtomicCounterBufferSize                = 16384,
    .maxTransformFeedbackBuffers               = 4,
    .maxTransformFeedbackInterleavedComponents = 64,
    .maxCullDistances                          = 8,
    .maxCombinedClipAndCullDistances           = 8,
    .maxSamples                                = 4,
    .maxMeshOutputVerticesNV                   = 256,
    .maxMeshOutputPrimitivesNV                 = 512,
    .maxMeshWorkGroupSizeX_NV                  = 32,
    .maxMeshWorkGroupSizeY_NV                  = 1,
    .maxMeshWorkGroupSizeZ_NV                  = 1,
    .maxTaskWorkGroupSizeX_NV                  = 32,
    .maxTaskWorkGroupSizeY_NV                  = 1,
    .maxTaskWorkGroupSizeZ_NV                  = 1,
    .maxMeshViewCountNV                        = 4,
    .limits{
        .nonInductiveForLoops                 = 1,
        .whileLoops                           = 1,
        .doWhileLoops                         = 1,
        .generalUniformIndexing               = 1,
        .generalAttributeMatrixVectorIndexing = 1,
        .generalVaryingIndexing               = 1,
        .generalSamplerIndexing               = 1,
        .generalVariableIndexing              = 1,
        .generalConstantMatrixVectorIndexing  = 1,
    }};

std::optional<std::vector<uint32_t>> Shader::compile_module(const std::string& file_name, const std::string& shader_code, EShaderStage shader_kind, bool optimize)
{
    BEGIN_NAMED_RECORD(COMPILE_SHADER_MODULE);

    glslang_stage_t shader_stage = GLSLANG_STAGE_COUNT;
    switch (shader_kind)
    {
    case EShaderStage::VertexShader:
        shader_stage = GLSLANG_STAGE_VERTEX;
        break;
    case EShaderStage::FragmentShader:
        shader_stage = GLSLANG_STAGE_FRAGMENT;
        break;
    case EShaderStage::GeometryShader:
        shader_stage = GLSLANG_STAGE_GEOMETRY;
        break;
    default:
        LOG_FATAL("unhandled shader kind");
    }

    const glslang_input_t input = {
        .language                          = GLSLANG_SOURCE_GLSL,
        .stage                             = shader_stage,
        .client                            = GLSLANG_CLIENT_VULKAN,
        .client_version                    = GLSLANG_TARGET_VULKAN_1_2,
        .target_language                   = GLSLANG_TARGET_SPV,
        .target_language_version           = GLSLANG_TARGET_SPV_1_3,
        .code                              = shader_code.c_str(),
        .default_version                   = 100,
        .default_profile                   = GLSLANG_NO_PROFILE,
        .force_default_version_and_profile = false,
        .forward_compatible                = false,
        .messages                          = GLSLANG_MSG_DEFAULT_BIT,
        .resource                          = reinterpret_cast<const glslang_resource_t*>(&glslang_default_resource),
    };

    glslang_shader_t* shader = glslang_shader_create(&input);

    if (!glslang_shader_preprocess(shader, &input))
    {
        LOG_ERROR("failed to compile shader : %s", glslang_shader_get_info_log(shader));
        return std::optional<std::vector<uint32_t>>();
        // use glslang_shader_get_info_log() and glslang_shader_get_info_debug_log()
    }

    if (!glslang_shader_parse(shader, &input))
    {
        LOG_ERROR("failed to compile shader : %s", glslang_shader_get_info_log(shader));
        return std::optional<std::vector<uint32_t>>();
        // use glslang_shader_get_info_log() and glslang_shader_get_info_debug_log()
    }

    glslang_program_t* program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
    {
        LOG_ERROR("failed to compile shader : %s", glslang_shader_get_info_log(shader));
        return std::optional<std::vector<uint32_t>>();
        // use glslang_program_get_info_log() and glslang_program_get_info_debug_log();
    }

    glslang_program_SPIRV_generate(program, input.stage);

    if (glslang_program_SPIRV_get_messages(program))
    {
        LOG_INFO("%s", glslang_program_SPIRV_get_messages(program));
    }

    glslang_shader_delete(shader);
    auto program_data = std::vector<uint32_t>(glslang_program_SPIRV_get_ptr(program), glslang_program_SPIRV_get_ptr(program) + glslang_program_SPIRV_get_size(program));

    glslang_program_delete(program);

    return program_data;
}

std::optional<VkShaderModule> Shader::create_shader_module(VkDevice logical_device, const std::vector<uint32_t>& bytecode)
{
    VkShaderModule shader_module;

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bytecode.size() * sizeof(uint32_t);
    createInfo.pCode    = reinterpret_cast<const uint32_t*>(bytecode.data());
    VkResult res        = vkCreateShaderModule(logical_device, &createInfo, vulkan_common::allocation_callback, &shader_module);
    if (!res == VK_SUCCESS)
    {
        LOG_ERROR("Failed to create shader module : %d", static_cast<uint32_t>(res));
        return nullptr;
    }
    return shader_module;
}

void Shader::build_reflection_data(const std::vector<uint32_t>& bytecode)
{
    const spirv_cross::Compiler compiler(bytecode);

    /**
     * ENTRY POINTS
     */
    if (compiler.get_entry_points_and_stages().empty())
    {
        LOG_ERROR("shaders doesn't contains entry point");
    }
    else
    {
        entry_point = compiler.get_entry_points_and_stages()[0].name;
    }

    if (const auto push_constant_buffers = compiler.get_shader_resources().push_constant_buffers; !push_constant_buffers.empty())
    {
        if (push_constant_buffers.size() != 1)
        {
            LOG_ERROR("cannot handle more than one push constant buffer per shader");
        }
        else
        {
            const auto push_constant_buffer = push_constant_buffers[0];
            auto       var_type             = compiler.get_type(push_constant_buffer.type_id);

            push_constants = ShaderProperty{.property_name  = push_constant_buffer.name,
                                            .property_type  = var_type.basetype,
                                            .structure_size = compiler.get_declared_struct_size(var_type),
                                            .location       = 0,
                                            .vec_size       = var_type.vecsize,
                                            .shader_stage   = shader_stage};
            for (int i = 0; i < var_type.member_types.size(); ++i)
            {
                const auto member_type = compiler.get_type(var_type.member_types[i]);
                push_constants->structure_properties.emplace_back(ShaderProperty{.property_name  = compiler.get_member_name(var_type.type_alias, i).c_str(),
                                                                                 .property_type  = member_type.basetype,
                                                                                 .structure_size = 0,
                                                                                 // compiler.get_declared_struct_size(member_type),
                                                                                 .location     = 0,
                                                                                 .vec_size     = member_type.vecsize,
                                                                                 .shader_stage = shader_stage});
            }
        }
    }

    /**
     *  UNIFORM BUFFERS
     */


    for (const auto& buffer : compiler.get_shader_resources().uniform_buffers)
    {
        auto var_type = compiler.get_type(buffer.type_id);

        ShaderProperty new_buffer = ShaderProperty{.property_name  = buffer.name,
                                                   .property_type  = var_type.basetype,
                                                   .structure_size = compiler.get_declared_struct_size(var_type),
                                                   .location       = compiler.get_decoration(buffer.id, spv::DecorationBinding),
                                                   .vec_size       = var_type.vecsize,
                                                   .shader_stage   = shader_stage};
        for (int i = 0; i < var_type.member_types.size(); ++i)
        {
            const auto member_type = compiler.get_type(var_type.member_types[i]);
            new_buffer.structure_properties.emplace_back(ShaderProperty{.property_name  = compiler.get_member_name(var_type.type_alias, i).c_str(),
                                                                        .property_type  = member_type.basetype,
                                                                        .structure_size = 0,
                                                                        // compiler.get_declared_struct_size(member_type),
                                                                        .location     = 0,
                                                                        .vec_size     = member_type.vecsize,
                                                                        .shader_stage = shader_stage});
        }
        uniform_buffer.emplace_back(new_buffer);
    }

    /**
     * STORAGE BUFFER
     */


    for (const auto& buffer : compiler.get_shader_resources().storage_buffers)
    {
        auto var_type = compiler.get_type(buffer.type_id);

        ShaderProperty new_buffer = ShaderProperty{.property_name  = buffer.name,
                                                   .property_type  = var_type.basetype,
                                                   .structure_size = compiler.get_declared_struct_size(var_type),
                                                   .location       = compiler.get_decoration(buffer.id, spv::DecorationBinding),
                                                   .vec_size       = var_type.vecsize,
                                                   .shader_stage   = shader_stage};
        for (int i = 0; i < var_type.member_types.size(); ++i)
        {
            const auto member_type = compiler.get_type(var_type.member_types[i]);
            new_buffer.structure_properties.emplace_back(ShaderProperty{.property_name  = compiler.get_member_name(var_type.type_alias, i).c_str(),
                                                                        .property_type  = member_type.basetype,
                                                                        .structure_size = 0,
                                                                        // compiler.get_declared_struct_size(member_type),
                                                                        .location     = 0,
                                                                        .vec_size     = member_type.vecsize,
                                                                        .shader_stage = shader_stage});
        }
        storage_buffer.emplace_back(new_buffer);
    }


    /**
     * SAMPLED IMAGES
     */

    for (const auto& image : compiler.get_shader_resources().sampled_images)
    {
        auto var_type = compiler.get_type(image.type_id);
        sampled_images.emplace_back(ShaderProperty{.property_name  = image.name,
                                                   .property_type  = var_type.basetype,
                                                   .structure_size = 0,
                                                   .location       = compiler.get_decoration(image.id, spv::DecorationBinding),
                                                   .vec_size       = var_type.vecsize,
                                                   .shader_stage   = shader_stage});
    }

#if ENABLE_SHADER_LOGGING
    std::string shader_log;

    shader_log += stringutils::format("interface variables : %d\n", compiler.get_active_interface_variables().size());
    for (const auto& var : compiler.get_active_interface_variables())
    {
        auto& type = compiler.get_type_from_variable(var);
        shader_log += stringutils::format("\t-- %s %s\n", magic_enum::enum_name(type.basetype).data(), compiler.get_remapped_declared_block_name(var).c_str());
    }

    if (push_constants)
    {
        shader_log += stringutils::format("push constant : %s%d %s (%d bytes)\n", magic_enum::enum_name(push_constants->property_type).data(), push_constants->vec_size, push_constants->property_name.c_str(),
                                          push_constants->structure_size);
        for (const auto& struct_member : push_constants->structure_properties)
        {
            shader_log += stringutils::format("\t-- %s%d %s\n", magic_enum::enum_name(struct_member.property_type).data(), struct_member.vec_size, struct_member.property_name.c_str());
        }
    }

    if (!sampled_images.empty())
    {
        shader_log += stringutils::format("sampled images : %d\n", sampled_images.size());
        for (const auto& image : sampled_images)
        {
            shader_log += stringutils::format("\t-- %s%d %s binding=%d\n", magic_enum::enum_name(image.property_type).data(), image.vec_size, image.property_name.c_str(), image.location);
        }
    }

    if (!uniform_buffer.empty())
    {
        shader_log += stringutils::format("uniform buffers : %d\n", uniform_buffer.size());
        for (const auto& buffer : uniform_buffer)
        {
            shader_log += stringutils::format("\t-- %s%d %s (%d bytes) binding=%d\n", magic_enum::enum_name(buffer.property_type).data(), buffer.vec_size, buffer.property_name.c_str(), buffer.structure_size, buffer.location);
            for (const auto& struct_member : buffer.structure_properties)
            {
                shader_log += stringutils::format("\t---- %s%d %s\n", magic_enum::enum_name(struct_member.property_type).data(), struct_member.vec_size, struct_member.property_name.c_str());
            }
        }
    }

    if (!storage_buffer.empty())
    {
        shader_log += stringutils::format("storage buffers : %d\n", storage_buffer.size());
        for (const auto& buffer : storage_buffer)
        {
            shader_log +=
                stringutils::format("\t-- %s%d %s (%d bytes) binding=%d\n", magic_enum::enum_name(buffer.property_type).data(), buffer.vec_size, buffer.property_name.c_str(), buffer.structure_size, buffer.location);
            for (const auto& struct_member : buffer.structure_properties)
            {
                shader_log += stringutils::format("\t---- %s%d %s\n", magic_enum::enum_name(struct_member.property_type).data(), struct_member.vec_size, struct_member.property_name.c_str());
            }
        }
    }

    LOG_DEBUG("\ncompiling shader module [%s] : entry point = '%s'\n%s", get_id().to_string().c_str(), entry_point.c_str(), shader_log.c_str());
#endif
}
