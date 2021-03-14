

#include "rendering/framegraph/framegraph.h"


#include <vulkan/vulkan_core.h>



#include "jobSystem/job_system.h"
#include "rendering/window.h"
#include "rendering/vulkan/commandPool.h"

void FramegraphSubpass::render()
{

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = 0; // Optional
	begin_info.pInheritanceInfo = nullptr; // Optional

	const VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	
	VK_ENSURE(vkBeginCommandBuffer(command_buffer, &begin_info), "Failed to create command buffer for graph task '%s'", task_name.c_str());
	
	VkRenderPassBeginInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = render_pass;
	render_pass_info.framebuffer = framebuffer.buffer;
	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = VkExtent2D{ framebuffer.width, framebuffer.height };
	render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
	render_pass_info.pClearValues = clear_values.data();

	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);


	/**
	 *  DRAW CONTENT
	 */


	

	vkCmdEndRenderPass(command_buffer);
	VK_ENSURE(vkEndCommandBuffer(command_buffer), "Failed to register command buffer for graph task '%s'", task_name.c_str());	
}

void FramegraphSubpass::create_subpass()
{
	VkAttachmentDescription subpass_attachment{};
	subpass_attachment.format = framebuffer.image_format;
	subpass_attachment.samples = pass_samples;
	subpass_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	subpass_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	subpass_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	subpass_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	subpass_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	subpass_attachment.finalLayout = framebuffer.is_depth_stencil_buffer ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	

	
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = vulkan_utils::get_depth_format(context->physical_device);
	depthAttachment.samples = pass_samples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = msaa_sample_count > 1 ? &colorAttachmentResolveRef : nullptr;
	subpass.inputAttachmentCount = 0;                            // Input attachments can be used to sample from contents of a previous subpass
	subpass.pInputAttachments = nullptr;                         // (Input attachments not used by this example)
	subpass.preserveAttachmentCount = 0;                         // Preserved attachments can be used to loop (and preserve) attachments through subpasses
	subpass.pPreserveAttachments = nullptr;                      // (Preserve attachments not used by this example)
}

void FramegraphPass::render()
{
	for (auto& task : group_tasks)
	{
		job_system::new_job([&task] {
			task.render();
		});
	}
	job_system::wait_children();
}



void FramegraphPass::create_or_recreate_render_pass()
{
	logger_log("create render pass group '%s'", group_name.c_str());

	
	if (render_pass != VK_NULL_HANDLE) vkDestroyRenderPass(context->get_context()->logical_device, render_pass, vulkan_common::allocation_callback);



	logger_log("Create render pass");
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapchain_surface_format.format;
	colorAttachment.samples = msaa_sample_count;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = max_msaa_sample_count > 1 ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Used in color attachment or presented to KHR

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = vulkan_utils::get_depth_format(context->physical_device);
	depthAttachment.samples = msaa_sample_count;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = swapchain_surface_format.format;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef{};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = msaa_sample_count > 1 ? &colorAttachmentResolveRef : nullptr;
	subpass.inputAttachmentCount = 0;                            // Input attachments can be used to sample from contents of a previous subpass
	subpass.pInputAttachments = nullptr;                         // (Input attachments not used by this example)
	subpass.preserveAttachmentCount = 0;                         // Preserved attachments can be used to loop (and preserve) attachments through subpasses
	subpass.pPreserveAttachments = nullptr;                      // (Preserve attachments not used by this example)

	std::array<VkSubpassDependency, 2> dependencies;
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;                             // Producer of the dependency
	dependencies[0].dstSubpass = 0;                                               // Consumer is our single subpass that will wait for the execution depdendency
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Match our pWaitDstStageMask when we vkQueueSubmit
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // is a loadOp stage for color attachments
	dependencies[0].srcAccessMask = 0;                                            // semaphore wait already does memory dependency for us
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;         // is a loadOp CLEAR access mask for color attachments
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;                                               // Producer of the dependency is our single subpass
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;                             // Consumer are all commands outside of the renderpass
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // is a storeOp stage for color attachments
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;          // Do not block any subsequent work
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;         // is a storeOp `STORE` access mask for color attachments
	dependencies[1].dstAccessMask = 0;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	std::vector<VkAttachmentDescription> attachments;

	if (msaa_sample_count > 1)
	{
		attachments.push_back(colorAttachment);
		attachments.push_back(depthAttachment);
		attachments.push_back(colorAttachmentResolve);
	}
	else
	{
		attachments.push_back(colorAttachment);
		attachments.push_back(depthAttachment);
	}
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();
	VK_ENSURE(vkCreateRenderPass(context->logical_device, &renderPassInfo, vulkan_common::allocation_callback, &render_pass), "Failed to create render pass");

	
}

void Framegraph::render()
{
	for (auto& group : graph_groups)
	{
		group.render();
	}
}
