
#include "rendering/framegraph/renderpass.h"

#include <array>


#include "ios/logger.h"
#include "jobSystem/job_system.h"
#include "rendering/window.h"
#include "rendering/framegraph/framegraph.h"
#include "rendering/framegraph/subpass.h"
#include "rendering/vulkan/commandPool.h"

FramegraphPass::FramegraphPass(VkExtent2D resolution, const std::string& in_pass_name, const std::vector<std::string>& in_dependencies, std::vector<FramegraphSubpass> in_subpasses)
	: subpasses(in_subpasses), pass_name(in_pass_name), size(resolution), dependencies_names(in_dependencies) {}

void FramegraphPass::render(DrawInfo draw_info)
{
	draw_info.command_buffer = command_buffers[draw_info.image_index];


	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = 0; // Optional
	begin_info.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(draw_info.command_buffer, &begin_info) != VK_SUCCESS) { logger_fail("Failed to create command buffer for pass #%s", pass_name.c_str()); }

	std::array<VkClearValue, 2> clear_values{};
	clear_values[0].color = { 0.6f, 0.9f, 1.f, 1.0f };
	clear_values[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = render_pass;
	render_pass_info.framebuffer = pass_framebuffer;
	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = size;
	render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
	render_pass_info.pClearValues = clear_values.data();

	vkCmdBeginRenderPass(draw_info.command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	// Set viewport and scissor params
	VkViewport viewport;
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = static_cast<float>(size.width);
	viewport.height = static_cast<float>(size.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor;
	scissor.extent = size;
	scissor.offset = VkOffset2D{ 0, 0 };
	vkCmdSetViewport(draw_info.command_buffer, 0, 1, &viewport);
	vkCmdSetScissor(draw_info.command_buffer, 0, 1, &scissor);




	// RECORD COMMANDS HERE






	vkCmdEndRenderPass(draw_info.command_buffer);
	VK_ENSURE(vkEndCommandBuffer(draw_info.command_buffer), "Failed to register command buffer for pass #s", pass_name.c_str());





	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore acquire_wait_semaphore[] = { image_acquire_semaphores[draw_info.frame_id] };
	VkPipelineStageFlags wait_stage[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = acquire_wait_semaphore;
	submitInfo.pWaitDstStageMask = wait_stage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &draw_info.command_buffer;

	VkSemaphore finished_semaphore[] = { render_finished_semaphores[draw_info.frame_id] }; // This fence is used to tell when the gpu can present the submitted data
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = finished_semaphore;

	/** Submit command buffers */
	vkResetFences(parent->context->get_context()->logical_device, 1, &in_flight_fences[draw_info.frame_id]);
	parent->context->get_context()->submit_graphic_queue(submitInfo, in_flight_fences[draw_info.frame_id]); // Pass fence to know when all the data are submitted
}

void FramegraphPass::init(Framegraph* in_parent)
{
	parent = in_parent;

	create_render_pass();
}

void FramegraphPass::create_frame_objects()
{
	// Create command buffers
	command_buffers.resize(parent->context->get_image_count());
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = parent->context->get_command_pool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
	VK_ENSURE(vkAllocateCommandBuffers(parent->context->get_context()->logical_device, &allocInfo, command_buffers.data()), "Failed to allocate command buffer");



	
}


void FramegraphPass::create_render_pass()
{
	for (auto& subpass : subpasses)
	{
		subpass.build_buffer(this);
	}
	create_frame_objects();

	logger_log("create render pass group '%s'", pass_name.c_str());
	if (render_pass != VK_NULL_HANDLE) logger_fail("cannot recreate render pass yet");

	/*
	 *  QUERY SUBPASSES
	 */

	std::vector<VkAttachmentDescription> attachment_descriptions;
	std::vector<VkAttachmentReference> color_attachment_references;
	std::optional<VkAttachmentReference> depth_stencil_attachment_reference;

	for (int i = 0; i < subpasses.size(); ++i)
	{
		attachment_descriptions.push_back(subpasses[i].get_attachment_description());
		if (subpasses[i].framebuffer->is_depth_stencil_buffer)
		{
			if (depth_stencil_attachment_reference) logger_error("cannot handle multiple depth attachment per render pass");
			depth_stencil_attachment_reference = VkAttachmentReference{
				.attachment = static_cast<uint32_t>(i),
				.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			};
		}
		else {
			color_attachment_references.push_back(VkAttachmentReference{
				.attachment = static_cast<uint32_t>(i),
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				});
		}
	}

	VkSubpassDescription subpass_description = {};
	subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_description.pColorAttachments = color_attachment_references.data();
	subpass_description.colorAttachmentCount = static_cast<uint32_t>(color_attachment_references.size());
	subpass_description.pDepthStencilAttachment = depth_stencil_attachment_reference.has_value() ? &depth_stencil_attachment_reference.value() : nullptr;

	/*
	 *  SETUP SUBPASSES DEPENDENCIES
	 */

	 // Use subpass dependencies_names for attachment layout transitions
	std::array<VkSubpassDependency, 2> dependencies = {};

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attachment_descriptions.data();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachment_descriptions.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass_description;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies.data();
	VK_ENSURE(vkCreateRenderPass(parent->context->get_context()->logical_device, &renderPassInfo, vulkan_common::allocation_callback, &render_pass), "failed to create render pass");

	std::vector<VkImageView> render_pass_attachments;

	for (const auto& pass : subpasses)
	{
		render_pass_attachments.push_back(pass.framebuffer->buffer_view);
	}

	VkFramebufferCreateInfo fbufCreateInfo = {};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.pNext = NULL;
	fbufCreateInfo.renderPass = render_pass;
	fbufCreateInfo.pAttachments = render_pass_attachments.data();
	fbufCreateInfo.attachmentCount = static_cast<uint32_t>(render_pass_attachments.size());
	fbufCreateInfo.width = size.width;
	fbufCreateInfo.height = size.height;
	fbufCreateInfo.layers = 1;
	fbufCreateInfo.pNext = nullptr;
	VK_ENSURE(vkCreateFramebuffer(parent->context->get_context()->logical_device, &fbufCreateInfo, nullptr, &pass_framebuffer));

	// Create sampler to sample from the color attachments
	VkSamplerCreateInfo sampler;
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.magFilter = VK_FILTER_NEAREST;
	sampler.minFilter = VK_FILTER_NEAREST;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	sampler.flags = 0;
	sampler.anisotropyEnable = VK_TRUE;
	sampler.unnormalizedCoordinates = VK_FALSE;
	sampler.compareEnable = VK_FALSE;
	sampler.pNext = nullptr;
	VK_ENSURE(vkCreateSampler(parent->context->get_context()->logical_device, &sampler, nullptr, &color_sampler));


}