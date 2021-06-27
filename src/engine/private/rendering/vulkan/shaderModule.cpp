
#include "rendering/vulkan/shaderModule.h"

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vulkan/vulkan_core.h>

#include <cpputils/logger.hpp>

#include "StandAlone/ResourceLimits.h"
#include "glslang/Include/glslang_c_interface.h"
#include "glslang/Include/glslang_c_shader_types.h"
#include "rendering/vulkan/common.h"
#include "spirv_glsl.hpp"
#include "ui/window/windows/profiler.h"

#define ENABLE_SHADER_LOGGING true

std::optional<std::string> read_shader_file(const std::filesystem::path& source_path)
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

inline static bool is_glslang_initialized = false;

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

std::optional<std::vector<uint32_t>> compile_module(const std::string& file_name, const std::string& shader_code, glslang_stage_t shader_kind, bool optimize = true)
{
    BEGIN_NAMED_RECORD(COMPILE_SHADER_MODULE);

    if (!is_glslang_initialized)
        LOG_FATAL("GLSLANG IS NOT INITIALIZED. Please call ShaderModule::initialize_glslang()");

    const glslang_input_t input = {
        .language                          = GLSLANG_SOURCE_GLSL,
        .stage                             = shader_kind,
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
        LOG_ERROR("failed to compile shader : %s", glslang_shader_get_info_debug_log(shader));
        return std::optional<std::vector<uint32_t>>();
        // use glslang_shader_get_info_log() and glslang_shader_get_info_debug_log()
    }

    if (!glslang_shader_parse(shader, &input))
    {
        LOG_ERROR("failed to compile shader : %s", glslang_shader_get_info_debug_log(shader));
        return std::optional<std::vector<uint32_t>>();
        // use glslang_shader_get_info_log() and glslang_shader_get_info_debug_log()
    }

    glslang_program_t* program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
    {
        LOG_ERROR("failed to compile shader : %s", glslang_shader_get_info_debug_log(shader));
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

std::optional<VkShaderModule> create_shader_module(VkDevice logical_device, const std::vector<uint32_t>& bytecode)
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

ShaderModule::ShaderModule(VkDevice logical_device, std::string in_file_name, const std::string& shader_code, glslang_stage_t shader_kind) : device(logical_device), file_name(in_file_name)
{
    if (auto bytecode = compile_module(file_name, shader_code, shader_kind))
    {
        if (auto shader_m = create_shader_module(logical_device, bytecode.value()))
        {
            build_reflection_data(bytecode.value());
            shader_module = shader_m.value();
        }
    }
}

ShaderModule::ShaderModule(VkDevice logical_device, std::filesystem::path source_path, glslang_stage_t shader_kind) : device(logical_device), file_name(source_path.string())
{
    auto shader_data = read_shader_file(source_path);
    if (shader_data)
    {
        if (auto bytecode = compile_module(source_path.string(), shader_data.value(), shader_kind))
        {
            if (auto shader_m = create_shader_module(logical_device, bytecode.value()))
            {
                build_reflection_data(bytecode.value());
                shader_module = shader_m.value();
            }
        }
    }
}

ShaderModule::~ShaderModule()
{
    vkDestroyShaderModule(device, shader_module, vulkan_common::allocation_callback);
}

void ShaderModule::build_reflection_data(const std::vector<uint32_t>& bytecode)
{
    const spirv_cross::Compiler compiler(bytecode);

#if ENABLE_SHADER_LOGGING
    std::string shader_log = "\n### NEW SHADER ###\n";
#endif

    /**
     *  PUSH CONSTANTS
     */

    const auto push_constant_buffers = compiler.get_shader_resources().push_constant_buffers;

#if ENABLE_SHADER_LOGGING
    shader_log += stringutils::format("push constant : %d\n", push_constant_buffers.size());
    for (auto& push_constant : push_constant_buffers)
    {
        shader_log += stringutils::format("\t-- #%d (%s)\n", push_constant.id, push_constant.name.c_str());
    }
#endif

    push_constant_buffer_size = 0;
    if (!push_constant_buffers.empty())
    {
        if (push_constant_buffers.size() != 1)
            LOG_ERROR("unhandled push constant buffer count");

        for (auto& buffer_range : compiler.get_active_buffer_ranges(compiler.get_shader_resources().push_constant_buffers[0].id))
        {
#if ENABLE_SHADER_LOGGING
            shader_log += stringutils::format("\t\t ---- Accessing member # % u, offset % u, size %u\n", buffer_range.index, buffer_range.offset, buffer_range.range);
#endif
            push_constant_buffer_size += static_cast<uint32_t>(buffer_range.range);
        }

#if ENABLE_SHADER_LOGGING
        shader_log += stringutils::format("push constant size : %d\n", push_constant_buffer_size);
#endif
    }

    /**
     *  UNIFORM BUFFERS
     */

    const auto uniform_buffers = compiler.get_shader_resources().uniform_buffers;

#if ENABLE_SHADER_LOGGING
    shader_log += stringutils::format("uniform buffers : %d\n", uniform_buffers.size());
#endif

    if (!uniform_buffers.empty())
    {
        if (uniform_buffers.size() != 1)
            LOG_ERROR("unhandled uniform buffer count");
        for (auto& buffer : uniform_buffers)
        {
            uint32_t set           = compiler.get_decoration(buffer.id, spv::DecorationDescriptorSet);
            uniform_buffer_binding = compiler.get_decoration(buffer.id, spv::DecorationBinding);
#if ENABLE_SHADER_LOGGING
            shader_log += stringutils::format("\t -- Found UBO % s at set = %u, binding = %u!\n", buffer.name.c_str(), set, uniform_buffer_binding);
#endif
        }
    }

    /**
     *  IMAGE SAMPLERS
     */

    const auto sampled_image = compiler.get_shader_resources().sampled_images;

#if ENABLE_SHADER_LOGGING
    shader_log += stringutils::format("uniform sampler : %d\n", sampled_image.size());
#endif

    if (!sampled_image.empty())
    {
        for (const auto& sampler : sampled_image)
        {
            uint32_t set                   = compiler.get_decoration(sampler.id, spv::DecorationDescriptorSet);
            uint32_t sampled_image_binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);
#if ENABLE_SHADER_LOGGING
            shader_log += stringutils::format("\t -- Found sampled image % s at set = % u, binding = % u!\n", sampler.name.c_str(), set, sampled_image_binding);
#endif
            sampled_image_bindings.push_back(sampled_image_binding);
        }
    }
#if ENABLE_SHADER_LOGGING
    LOG_DEBUG(shader_log.c_str());
#endif
}

void ShaderModule::initialize_glslang()
{
    is_glslang_initialized = true;
    glslang_initialize_process();
}

void ShaderModule::shutdown_glslang()
{
    is_glslang_initialized = false;
    glslang_finalize_process();
}
