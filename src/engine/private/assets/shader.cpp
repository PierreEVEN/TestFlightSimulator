
#include "assets/shader.h"

#include <fstream>
#include <optional>

#include "assets/staticMesh.h"
#include "glm/glm.hpp"



#include "jobSystem/job_system.h"
#include "rendering/window.h"
#include "rendering/vulkan/descriptorPool.h"
#include "ui/window/windows/profiler.h"


Shader::Shader(const std::filesystem::path& vertex_shader_path,	const std::filesystem::path& fragment_shader_path, const std::filesystem::path& geometry_shader_path)
{
	shader_creation_job = job_system::new_job([&, vertex_shader_path, fragment_shader_path, geometry_shader_path] {

		std::shared_ptr<job_system::IJobTask> shader_creation_vertex;
		std::shared_ptr<job_system::IJobTask> shader_creation_fragment;
		std::shared_ptr<job_system::IJobTask> shader_creation_geometry;
		
		if (!vertex_shader_path.empty()) shader_creation_vertex = job_system::new_job([&, vertex_shader_path]
			{
				BEGIN_NAMED_RECORD(CREATE_SHADER);
				vertex_module = (std::make_shared<ShaderModule>(window_context->get_context()->logical_device, vertex_shader_path, shaderc_vertex_shader));

			});
		if (!fragment_shader_path.empty()) shader_creation_fragment = job_system::new_job([&, fragment_shader_path]
			{
				BEGIN_NAMED_RECORD(CREATE_SHADER);
				fragment_module = (std::make_shared<ShaderModule>(window_context->get_context()->logical_device, fragment_shader_path, shaderc_fragment_shader));

			});
		if (!geometry_shader_path.empty()) shader_creation_geometry = job_system::new_job([&, geometry_shader_path]
			{
				BEGIN_NAMED_RECORD(CREATE_SHADER);
				geometry_module = (std::make_shared<ShaderModule>(window_context->get_context()->logical_device, geometry_shader_path, shaderc_geometry_shader));

			});


		if (shader_creation_vertex) shader_creation_vertex->wait();
		if (shader_creation_fragment) shader_creation_fragment->wait();
		if (shader_creation_geometry) shader_creation_geometry->wait();

		create_descriptor_sets(create_layout_bindings());
		create_pipeline();
		
		logger_log("loaded shader %s", asset_id->to_string().c_str());
		});
}

Shader::~Shader()
{
	shader_creation_job->wait();
	destroy();
}

void Shader::create_pipeline()
{
	/** Shader pipeline */
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	if (vertex_module) {
		if (vertex_module->get() == VK_NULL_HANDLE) {
			logger_error("invalid vertex stage : %s", asset_id->to_string().c_str());
			return;
		}
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertex_module->get();
		vertShaderStageInfo.pName = "main";
		shaderStages.push_back(vertShaderStageInfo);
	}

	if (geometry_module) {
		if (geometry_module->get() == VK_NULL_HANDLE) {
			logger_error("invalid geometry stage : %s", asset_id->to_string().c_str());
			return;
		}
		VkPipelineShaderStageCreateInfo geomShaderStageInfo{};
		geomShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		geomShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		geomShaderStageInfo.module = geometry_module->get();
		geomShaderStageInfo.pName = "main";
		shaderStages.push_back(geomShaderStageInfo);
	}

	if (fragment_module) {
		if (fragment_module->get() == VK_NULL_HANDLE) {
			logger_error("invalid fragment stage : %s", asset_id->to_string().c_str());
			return;
		}
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragment_module->get();
		fragShaderStageInfo.pName = "main";
		shaderStages.push_back(fragShaderStageInfo);
	}

	/** Model transform */
	VkPushConstantRange pushConstantRange;
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(glm::mat4);

	/** Pipeline layout */
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptor_set_layout;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	VK_ENSURE(vkCreatePipelineLayout(get_context()->get_context()->logical_device, &pipelineLayoutInfo, nullptr, &pipeline_layout), "Failed to create pipeline layout");

	auto bindingDescription = Vertex::get_binding_description();
	auto attributeDescriptions = Vertex::get_attribute_descriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.rasterizationSamples = static_cast<VkSampleCountFlagBits>(get_context()->get_msaa_sample_count());
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional
	multisampling.sampleShadingEnable = get_context()->get_msaa_sample_count() > 1 ? VK_TRUE : VK_FALSE;
	multisampling.minSampleShading = .2f;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState; // Optional
	pipelineInfo.layout = pipeline_layout;
	pipelineInfo.renderPass = get_context()->get_render_pass();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional
	pipelineInfo.pDepthStencilState = &depthStencil;
	VK_ENSURE(vkCreateGraphicsPipelines(get_context()->get_context()->logical_device, VK_NULL_HANDLE, 1, &pipelineInfo, vulkan_common::allocation_callback, &shader_pipeline), "Failed to create material graphic pipeline");
}

void Shader::create_descriptor_sets(std::vector<VkDescriptorSetLayoutBinding> layout_bindings)
{
	/** Create descriptor set layout */
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(layout_bindings.size());
	layoutInfo.pBindings = layout_bindings.data();
	VK_ENSURE(vkCreateDescriptorSetLayout(get_context()->get_context()->logical_device, &layoutInfo, vulkan_common::allocation_callback, &descriptor_set_layout), "Failed to create descriptor set layout");

	/** Allocate descriptor set */
	std::vector<VkDescriptorSetLayout> layouts(get_context()->get_image_count(), descriptor_set_layout);
	descriptor_sets.resize(get_context()->get_image_count());
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(get_context()->get_image_count());
	allocInfo.pSetLayouts = layouts.data();
	allocInfo.descriptorPool = VK_NULL_HANDLE;
	get_context()->get_descriptor_pool()->alloc_memory(allocInfo);
	VK_ENSURE(vkAllocateDescriptorSets(get_context()->get_context()->logical_device, &allocInfo, descriptor_sets.data()), "Failed to allocate descriptor sets");
}

std::vector<VkDescriptorSetLayoutBinding> Shader::create_layout_bindings()
{
	/*
	uint32_t currentId = 0;
	std::vector<VkDescriptorSetLayoutBinding> outBindings;

	if (materialProperties.bUseGlobalUbo) {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = currentId;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		outBindings.push_back(uboLayoutBinding);
		currentId++;
	}

	// Vertex textures 
	for (uint32_t i = 0; i < materialProperties.VertexTexture2DCount; ++i) {
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = currentId;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		outBindings.push_back(samplerLayoutBinding);
		currentId++;
	}

	// Fragment textures 
	for (uint32_t i = 0; i < materialProperties.FragmentTexture2DCount; ++i) {
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = currentId;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		outBindings.push_back(samplerLayoutBinding);
		currentId++;
	}
	return outBindings;
	*/
		return {};
}

void Shader::destroy()
{
	if (shader_pipeline != VK_NULL_HANDLE) vkDestroyPipeline(get_context()->get_context()->logical_device, shader_pipeline, vulkan_common::allocation_callback);
	if (pipeline_layout != VK_NULL_HANDLE) vkDestroyPipelineLayout(get_context()->get_context()->logical_device, pipeline_layout, vulkan_common::allocation_callback);
	if (descriptor_set_layout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(get_context()->get_context()->logical_device, descriptor_set_layout, vulkan_common::allocation_callback);
	shader_pipeline = VK_NULL_HANDLE;
	pipeline_layout = VK_NULL_HANDLE;
	descriptor_set_layout = VK_NULL_HANDLE;
}
