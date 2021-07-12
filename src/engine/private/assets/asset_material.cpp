

#include "assets/asset_material.h"

#include "assets/asset_mesh_data.h"
#include "assets/asset_shader.h"
#include "assets/asset_uniform_buffer.h"
#include "engine_interface.h"
#include "magic_enum/magic_enum.h"

/*
VkWriteDescriptorSet StructShaderParameter::generate_write_descriptor_sets()
{
    VkWriteDescriptorSet struct_descriptor_set{};
    struct_descriptor_set.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    struct_descriptor_set.dstSet          = descriptor_sets[iIndex];
    struct_descriptor_set.dstBinding      = currentBinding;
    struct_descriptor_set.dstArrayElement = 0;
    struct_descriptor_set.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    struct_descriptor_set.descriptorCount = 1;
    struct_descriptor_set.pBufferInfo     = &drawViewport->GetViewportUbos()->GetDescriptorBufferInfo(iIndex);
    struct_descriptor_set.pImageInfo      = nullptr;
    struct_descriptor_set.pNext           = nullptr;
    return struct_descriptor_set;
}
*/

Material::Material(const ShaderStageData& in_vertex_stage, const ShaderStageData& in_fragment_stage, const std::shared_ptr<PushConstant>& in_push_constant)
    : vertex_stage(in_vertex_stage), fragment_stage(in_fragment_stage), push_constant(in_push_constant)
{
    destroy_resources();
    if (!vertex_stage.shader || !fragment_stage.shader)
        return;
    create_descriptor_sets(make_layout_bindings());
    create_pipeline();
}

Material::~Material()
{
    destroy_resources();
}

void Material::update_push_constants(VkCommandBuffer& command_buffer)
{
    if (push_constant)
    {
        if (push_constant->get_data())
            vkCmdPushConstants(command_buffer, get_pipeline_layout(), push_constant->stage_flags, 0, static_cast<uint32_t>(push_constant->get_size()), push_constant->get_data());
        else
            LOG_ERROR("push constant data for vertex stage has never been set");
    }
}

void Material::destroy_resources()
{
    if (shader_pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(get_engine_interface()->get_gfx_context()->logical_device, shader_pipeline, vulkan_common::allocation_callback);
    if (pipeline_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(get_engine_interface()->get_gfx_context()->logical_device, pipeline_layout, vulkan_common::allocation_callback);
    if (descriptor_set_layout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(get_engine_interface()->get_gfx_context()->logical_device, descriptor_set_layout, vulkan_common::allocation_callback);
    shader_pipeline       = VK_NULL_HANDLE;
    pipeline_layout       = VK_NULL_HANDLE;
    descriptor_set_layout = VK_NULL_HANDLE;
}

std::vector<VkDescriptorSetLayoutBinding> Material::make_layout_bindings()
{
    vertex_uniform_bindings.clear();
    fragment_uniform_bindings.clear();

    std::vector<VkDescriptorSetLayoutBinding> result_bindings;

    // VERTEX BUFFER

    // vertex uniform buffers

    std::unordered_map<std::string, ShaderProperty> uniform_buffers;

    for (const auto& param : vertex_stage.shader->get_uniform_buffers())
    {
        uniform_buffers[param.property_name] = param;
    }

    for (const auto& uniform : vertex_stage.uniform_buffer)
    {
        if (const auto found_buffer = uniform_buffers.find(uniform->get_name()); found_buffer != uniform_buffers.end())
        {
            result_bindings.emplace_back(VkDescriptorSetLayoutBinding{
                .binding            = found_buffer->second.location,
                .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount    = 1,
                .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = nullptr,
            });
            vertex_uniform_bindings[found_buffer->second.property_name] = found_buffer->second.location;
        }
        else
        {
            LOG_ERROR("specified uniform buffer named %s that doesn't exist in vertex stage", uniform->get_name().c_str());
        }
    }

    uniform_buffers.clear();

    // FRAGMENT BUFFER

    // fragment uniform buffers

    for (const auto& param : fragment_stage.shader->get_uniform_buffers())
    {
        uniform_buffers[param.property_name] = param;
    }

    for (const auto& uniform : fragment_stage.uniform_buffer)
    {
        if (const auto found_buffer = uniform_buffers.find(uniform->get_name()); found_buffer != uniform_buffers.end())
        {
            result_bindings.emplace_back(VkDescriptorSetLayoutBinding{
                .binding            = found_buffer->second.location,
                .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount    = 1,
                .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = nullptr,
            });
            fragment_uniform_bindings[found_buffer->second.property_name] = found_buffer->second.location;
        }
        else
        {
            LOG_ERROR("specified uniform buffer named %s that doesn't exist in fragment stage", uniform->get_name().c_str());
        }
    }

    return result_bindings;
}

void Material::create_pipeline()
{
    if (pipeline_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(get_engine_interface()->get_gfx_context()->logical_device, pipeline_layout, vulkan_common::allocation_callback);
    if (shader_pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(get_engine_interface()->get_gfx_context()->logical_device, shader_pipeline, vulkan_common::allocation_callback);

    VK_CHECK(descriptor_set_layout, "Descriptor set layout should be initialized before graphic pipeline");
    VK_CHECK(get_engine_interface()->get_window()->get_render_pass(), "Render pass should be initialized before graphic pipeline");

    /** Shader pipeline */
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertex_stage.shader->get_shader_module();
    vertShaderStageInfo.pName  = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragment_stage.shader->get_shader_module();
    fragShaderStageInfo.pName  = "main";

    // PUSH CONSTANTS
    std::unique_ptr<VkPushConstantRange> push_constant_ranges;
    if (push_constant)
    {
        if (vertex_stage.shader->get_push_constants() || fragment_stage.shader->get_push_constants())
        {
            push_constant->stage_flags = vertex_stage.shader->get_push_constants() && fragment_stage.shader->get_push_constants() ? VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT
                                         : fragment_stage.shader->get_push_constants()                                            ? VK_SHADER_STAGE_FRAGMENT_BIT
                                                                                                                                  : VK_SHADER_STAGE_VERTEX_BIT;

            push_constant_ranges             = std::make_unique<VkPushConstantRange>();
            push_constant_ranges->stageFlags = push_constant->stage_flags;
            push_constant_ranges->offset     = 0;
            push_constant_ranges->size       = static_cast<uint32_t>(push_constant->get_size());
        }
        else
        {
            LOG_ERROR("specified push constant that is not available in current shaders");
        }
    }
    else if (vertex_stage.shader->get_push_constants() || fragment_stage.shader->get_push_constants())
    {
        LOG_ERROR("missing push constant parameter");
    }

    /** Pipeline layout */
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = 1;
    pipelineLayoutInfo.pSetLayouts            = &descriptor_set_layout;
    pipelineLayoutInfo.pushConstantRangeCount = push_constant_ranges ? 1 : 0;
    pipelineLayoutInfo.pPushConstantRanges    = push_constant_ranges ? push_constant_ranges.get() : nullptr;
    VK_ENSURE(vkCreatePipelineLayout(get_engine_interface()->get_gfx_context()->logical_device, &pipelineLayoutInfo, nullptr, &pipeline_layout), "Failed to create pipeline layout");

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingDescription    = Vertex::get_binding_description();
    auto attributeDescriptions = Vertex::get_attribute_descriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions      = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount  = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL; // VK_POLYGON_MODE_LINE : wireframe
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_FRONT_BIT; // VK_CULL_MODE_FRONT_BIT : backface culling
    rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp          = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor    = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples  = static_cast<VkSampleCountFlagBits>(get_engine_interface()->get_window()->get_msaa_sample_count());
    multisampling.minSampleShading      = 1.0f;     // Optional
    multisampling.pSampleMask           = nullptr;  // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable      = VK_FALSE; // Optional
    multisampling.sampleShadingEnable   = get_engine_interface()->get_window()->get_msaa_sample_count() > 1 ? VK_TRUE : VK_FALSE;
    multisampling.minSampleShading      = .2f;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable       = VK_TRUE; // DEPTH TEST
    depthStencil.depthWriteEnable      = VK_TRUE; // DEPTH TEST
    depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds        = 0.0f; // Optional
    depthStencil.maxDepthBounds        = 1.0f; // Optional
    depthStencil.stencilTestEnable     = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable         = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;      // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;      // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable     = VK_FALSE;
    colorBlending.logicOp           = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount   = 1;
    colorBlending.pAttachments      = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates    = dynamicStates;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = 2;
    pipelineInfo.pStages             = shaderStages;
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pDepthStencilState  = nullptr; // Optional
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = &dynamicState; // Optional
    pipelineInfo.layout              = pipeline_layout;
    pipelineInfo.renderPass          = get_engine_interface()->get_window()->get_render_pass();
    pipelineInfo.subpass             = 0;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex   = -1;             // Optional
    pipelineInfo.pDepthStencilState  = &depthStencil;
    VK_ENSURE(vkCreateGraphicsPipelines(get_engine_interface()->get_gfx_context()->logical_device, VK_NULL_HANDLE, 1, &pipelineInfo, vulkan_common::allocation_callback, &shader_pipeline),
              "Failed to create material graphic pipeline");
}

void Material::create_descriptor_sets(std::vector<VkDescriptorSetLayoutBinding> layoutBindings)
{
    /** Create descriptor set layout */
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutInfo.pBindings    = layoutBindings.data();
    VK_ENSURE(vkCreateDescriptorSetLayout(get_engine_interface()->get_gfx_context()->logical_device, &layoutInfo, vulkan_common::allocation_callback, &descriptor_set_layout), "Failed to create descriptor set layout");

    /** Allocate descriptor set */
    std::vector<VkDescriptorSetLayout> layouts(get_engine_interface()->get_window()->get_image_count(), descriptor_set_layout);
    descriptor_sets.resize(get_engine_interface()->get_window()->get_image_count());
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorSetCount = get_engine_interface()->get_window()->get_image_count();
    allocInfo.pSetLayouts        = layouts.data();
    allocInfo.descriptorPool     = VK_NULL_HANDLE;
    get_engine_interface()->get_window()->get_descriptor_pool()->alloc_memory(allocInfo);
    VK_ENSURE(vkAllocateDescriptorSets(get_engine_interface()->get_gfx_context()->logical_device, &allocInfo, descriptor_sets.data()), "Failed to allocate descriptor sets");
}

void Material::update_descriptor_sets(size_t imageIndex)
{
    std::vector<VkWriteDescriptorSet> write_descriptor_sets = {};

    for (const auto& uniform : vertex_stage.uniform_buffer)
    {
        if (auto binding = vertex_uniform_bindings.find(uniform->get_name()); binding != vertex_uniform_bindings.end())
        {
            write_descriptor_sets.emplace_back(VkWriteDescriptorSet{
                .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext            = nullptr,
                .dstSet           = descriptor_sets[imageIndex],
                .dstBinding       = binding->second,
                .dstArrayElement  = 0,
                .descriptorCount  = 1,
                .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImageInfo       = nullptr,
                .pBufferInfo      = uniform->get_descriptor_buffer_info(static_cast<uint32_t>(imageIndex)),
                .pTexelBufferView = nullptr,
            });
        }
        else
        {
            LOG_ERROR("failed to find binding for uniform buffer");
        }
    }

    for (const auto& uniform : fragment_stage.uniform_buffer)
    {
        if (auto binding = fragment_uniform_bindings.find(uniform->get_name()); binding != fragment_uniform_bindings.end())
        {
            write_descriptor_sets.emplace_back(VkWriteDescriptorSet{
                .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext            = nullptr,
                .dstSet           = descriptor_sets[imageIndex],
                .dstBinding       = binding->second,
                .dstArrayElement  = 0,
                .descriptorCount  = 1,
                .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImageInfo       = nullptr,
                .pBufferInfo      = uniform->get_descriptor_buffer_info(static_cast<uint32_t>(imageIndex)),
                .pTexelBufferView = nullptr,
            });
        }
        else
        {
            LOG_ERROR("failed to find binding for uniform buffer");
        }
    }

    /*
    std::vector<VkDescriptorImageInfo> vertexImageInfos(descriptor_sets.size());
    for (uint32_t i = 0; i < vertexImageInfos.size(); ++i)
    {
        vertexImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vertexImageInfos[i].imageView   = dynamicMaterialProperties.vertexTextures2D[i]->GetImageView();
        vertexImageInfos[i].sampler     = dynamicMaterialProperties.vertexTextures2D[i]->GetSampler();

        VkWriteDescriptorSet imgWDescSet{};
        imgWDescSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imgWDescSet.dstSet          = descriptor_sets[iIndex];
        imgWDescSet.dstBinding      = currentBinding;
        imgWDescSet.dstArrayElement = 0;
        imgWDescSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imgWDescSet.descriptorCount = 1;
        imgWDescSet.pImageInfo      = &vertexImageInfos[i];
        imgWDescSet.pBufferInfo     = nullptr;
        imgWDescSet.pNext           = nullptr;
        newDescSets.push_back(imgWDescSet);
        currentBinding++;
    }
    */
    
    vkUpdateDescriptorSets(get_engine_interface()->get_gfx_context()->logical_device, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
}