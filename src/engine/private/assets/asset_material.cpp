

#include "assets/asset_material.h"

#include "engine_interface.h"
#include "assets/asset_mesh_data.h"
#include "assets/asset_shader.h"

VkWriteDescriptorSet TextureShaderParameter::get_descriptor_set() const
{
    VkWriteDescriptorSet imgWDescSet{};
    /*
    fragmentImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    fragmentImageInfos[i].imageView   = dynamicMaterialProperties.fragmentTextures2D[i]->GetImageView();
    fragmentImageInfos[i].sampler     = dynamicMaterialProperties.fragmentTextures2D[i]->GetSampler();

    VkWriteDescriptorSet imgWDescSet{};
    imgWDescSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    imgWDescSet.dstSet          = parent->GetDescriptorSet(iIndex);
    imgWDescSet.dstBinding      = currentBinding;
    imgWDescSet.dstArrayElement = 0;
    imgWDescSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    imgWDescSet.descriptorCount = 1;
    imgWDescSet.pImageInfo      = &fragmentImageInfos[i];
    imgWDescSet.pBufferInfo     = nullptr;
    imgWDescSet.pNext           = nullptr;
    newDescSets.push_back(imgWDescSet);



*/
    return imgWDescSet;
}

Material::Material(const TAssetPtr<Shader>& in_vertex_shader, const TAssetPtr<Shader>& in_fragment_shader) : vertex_shader(in_vertex_shader), fragment_shader(in_fragment_shader)
{
    destroy_resources();
    create_descriptor_sets(MakeLayoutBindings({}));
    create_pipeline({});
}

Material::~Material()
{
    destroy_resources();
}

void Material::destroy_resources()
{
    if (shader_pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(get_engine_interface()->get_gfx_context()->logical_device, shader_pipeline, vulkan_common::allocation_callback);
    if (pipeline_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(get_engine_interface()->get_gfx_context()->logical_device, pipeline_layout, vulkan_common::allocation_callback);
    if (descriptorSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(get_engine_interface()->get_gfx_context()->logical_device, descriptorSetLayout, vulkan_common::allocation_callback);
    shader_pipeline      = VK_NULL_HANDLE;
    pipeline_layout      = VK_NULL_HANDLE;
    descriptorSetLayout = VK_NULL_HANDLE;
}

std::vector<VkDescriptorSetLayoutBinding> Material::MakeLayoutBindings(const SMaterialStaticProperties& materialProperties)
{
    uint32_t                                  currentId = 0;
    std::vector<VkDescriptorSetLayoutBinding> outBindings;

    if (materialProperties.bUseGlobalUbo)
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding            = currentId;
        uboLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount    = 1;
        uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        outBindings.push_back(uboLayoutBinding);
        currentId++;
    }

    /** Vertex textures */
    for (uint32_t i = 0; i < materialProperties.VertexTexture2DCount; ++i)
    {
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding            = currentId;
        samplerLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.descriptorCount    = 1;
        samplerLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        outBindings.push_back(samplerLayoutBinding);
        currentId++;
    }

    /** Fragment textures */
    for (uint32_t i = 0; i < materialProperties.FragmentTexture2DCount; ++i)
    {
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding            = currentId;
        samplerLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.descriptorCount    = 1;
        samplerLayoutBinding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        outBindings.push_back(samplerLayoutBinding);
        currentId++;
    }
    return outBindings;
}

void Material::create_pipeline(const SMaterialStaticProperties& materialProperties)
{

    if (pipeline_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(get_engine_interface()->get_gfx_context()->logical_device, pipeline_layout, vulkan_common::allocation_callback);
    if (shader_pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(get_engine_interface()->get_gfx_context()->logical_device, shader_pipeline, vulkan_common::allocation_callback);

    VK_CHECK(descriptorSetLayout, "Descriptor set layout should be initialized before graphic pipeline");
    VK_CHECK(get_engine_interface()->get_window()->get_render_pass(), "Render pass should be initialized before graphic pipeline");

    if (!vertex_shader)
    {
        LOG_ERROR("vertex shader is not valid");
        return;
    }

    if (!fragment_shader)
    {
        LOG_ERROR("fragment shader is not valid");
        return;
    }
    /** Shader pipeline */
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertex_shader->get_shader_module();
    vertShaderStageInfo.pName  = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragment_shader->get_shader_module();
    fragShaderStageInfo.pName  = "main";

    /** Model transform */
    VkPushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset     = 0;
    pushConstantRange.size       = sizeof(glm::dmat4);

    /** Pipeline layout */
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = 1;
    pipelineLayoutInfo.pSetLayouts            = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges    = &pushConstantRange;
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
    rasterizer.polygonMode             = materialProperties.materialCreationFlag & EMATERIAL_CREATION_FLAG_WIREFRAME ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = materialProperties.materialCreationFlag & EMATERIAL_CREATION_FLAG_DOUBLESIDED ? VK_CULL_MODE_NONE : VK_CULL_MODE_FRONT_BIT;
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
    depthStencil.depthTestEnable       = materialProperties.materialCreationFlag & EMATERIAL_CREATION_FLAG_DISABLE_DEPTH_TEST ? VK_FALSE : VK_TRUE;
    depthStencil.depthWriteEnable      = materialProperties.materialCreationFlag & EMATERIAL_CREATION_FLAG_DISABLE_DEPTH_TEST ? VK_FALSE : VK_TRUE;
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
    VK_ENSURE(vkCreateDescriptorSetLayout(get_engine_interface()->get_gfx_context()->logical_device, &layoutInfo, vulkan_common::allocation_callback, &descriptorSetLayout), "Failed to create descriptor set layout");

    /** Allocate descriptor set */
    std::vector<VkDescriptorSetLayout> layouts(get_engine_interface()->get_window()->get_image_count(), descriptorSetLayout);
    descriptor_sets.resize(get_engine_interface()->get_window()->get_image_count());
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(get_engine_interface()->get_window()->get_image_count());
    allocInfo.pSetLayouts        = layouts.data();
    allocInfo.descriptorPool     = VK_NULL_HANDLE;
    get_engine_interface()->get_window()->get_descriptor_pool()->alloc_memory(allocInfo);
    VK_ENSURE(vkAllocateDescriptorSets(get_engine_interface()->get_gfx_context()->logical_device, &allocInfo, descriptor_sets.data()), "Failed to allocate descriptor sets");
}

/*

void Rendering::MaterialRessourceItem::UpdateDescriptorSets(ViewportInstance* drawViewport, size_t imageIndex)
{
    if (dynamicMaterialProperties.vertexTextures2D.size() != staticMaterialProperties.VertexTexture2DCount)
    {
        LOG_ASSERT("Vertex texure number (" + ToString(dynamicMaterialProperties.vertexTextures2D.size()) + ") should be the same than material properties VertexTexture2DCount (" +
                   ToString(staticMaterialProperties.VertexTexture2DCount) + ")");
    }

    if (dynamicMaterialProperties.fragmentTextures2D.size() != staticMaterialProperties.FragmentTexture2DCount)
    {
        LOG_ASSERT("Fragment texure number (" + ToString(dynamicMaterialProperties.fragmentTextures2D.size()) + ") should be the same than material properties FragmentTexture2DCount (" +
                   ToString(staticMaterialProperties.FragmentTexture2DCount) + ")");
    }

    // for (int iIndex = 0; iIndex < G_SWAP_CHAIN_IMAGE_COUNT; ++iIndex) {
    size_t                            iIndex = imageIndex;
    std::vector<VkWriteDescriptorSet> newDescSets;
    uint32_t                          currentBinding = 0;

    if (staticMaterialProperties.bUseGlobalUbo)
    {

        VkWriteDescriptorSet matrixUbo{};
        matrixUbo.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        matrixUbo.dstSet          = parent->GetDescriptorSet(iIndex);
        matrixUbo.dstBinding      = currentBinding;
        matrixUbo.dstArrayElement = 0;
        matrixUbo.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        matrixUbo.descriptorCount = 1;
        matrixUbo.pBufferInfo     = &drawViewport->GetViewportUbos()->GetDescriptorBufferInfo(iIndex);
        matrixUbo.pImageInfo      = nullptr;
        matrixUbo.pNext           = nullptr;
        newDescSets.push_back(matrixUbo);
        currentBinding++;
    }

    std::vector<VkDescriptorImageInfo> vertexImageInfos(staticMaterialProperties.VertexTexture2DCount);
    for (uint32_t i = 0; i < staticMaterialProperties.VertexTexture2DCount; ++i)
    {
        vertexImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vertexImageInfos[i].imageView   = dynamicMaterialProperties.vertexTextures2D[i]->GetImageView();
        vertexImageInfos[i].sampler     = dynamicMaterialProperties.vertexTextures2D[i]->GetSampler();

        VkWriteDescriptorSet imgWDescSet{};
        imgWDescSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imgWDescSet.dstSet          = parent->GetDescriptorSet(iIndex);
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

    std::vector<VkDescriptorImageInfo> fragmentImageInfos(staticMaterialProperties.FragmentTexture2DCount);
    for (uint32_t i = 0; i < staticMaterialProperties.FragmentTexture2DCount; ++i)
    {
        fragmentImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        fragmentImageInfos[i].imageView   = dynamicMaterialProperties.fragmentTextures2D[i]->GetImageView();
        fragmentImageInfos[i].sampler     = dynamicMaterialProperties.fragmentTextures2D[i]->GetSampler();

        VkWriteDescriptorSet imgWDescSet{};
        imgWDescSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imgWDescSet.dstSet          = parent->GetDescriptorSet(iIndex);
        imgWDescSet.dstBinding      = currentBinding;
        imgWDescSet.dstArrayElement = 0;
        imgWDescSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imgWDescSet.descriptorCount = 1;
        imgWDescSet.pImageInfo      = &fragmentImageInfos[i];
        imgWDescSet.pBufferInfo     = nullptr;
        imgWDescSet.pNext           = nullptr;
        newDescSets.push_back(imgWDescSet);
        currentBinding++;
    }
    vkUpdateDescriptorSets(G_LOGICAL_DEVICE, static_cast<uint32_t>(newDescSets.size()), newDescSets.data(), 0, nullptr);
    //}
}
*/