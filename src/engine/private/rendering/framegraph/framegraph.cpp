

#include "rendering/framegraph/framegraph.h"



#include <queue>
#include <vulkan/vulkan_core.h>


#include "config.h"
#include "jobSystem/job_system.h"
#include "rendering/window.h"
#include "rendering/framegraph/framebuffer.h"
#include "rendering/framegraph/renderpass.h"
#include "rendering/vulkan/commandPool.h"
#include "rendering/vulkan/swapchain.h"

Framegraph::Framegraph(Window* in_context, const std::vector<std::shared_ptr<FramegraphPass>>& in_render_pass)
	: context(in_context)
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(context->get_handle(), &width, &height);
	swapchain = new Swapchain(VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) }, context);

	for (const auto& pass : in_render_pass)
	{
		graph_passes[pass->pass_name] = pass;
	}

	/*
	 * BUILD DEPENDENCY GRAPH
	 */
	for (auto& render_pass : graph_passes)
	{
		for (auto& dep : render_pass.second->dependencies_names)
		{
			for (auto& dep_pass : graph_passes)
			{
				if (dep_pass.second->pass_name == dep)
				{
					dep_pass.second->parent_pass = render_pass.second;
					render_pass.second->children_pass[dep_pass.second->pass_name] = dep_pass.second;
					render_pass.second->children_pass_list.push_back(dep_pass.second);
				}
			}
		}
	}
	for (auto& render_pass : graph_passes) {
		if (!render_pass.second->parent_pass) graph_top.push_back(render_pass.second);
	}

	/**
	 * INITIALIZE GRAPH PASSES
	 */
	for (auto& render_pass : graph_passes)
	{
		render_pass.second->init(this);
	}


	per_frame_data.resize(config::max_frame_in_flight);
	for (auto& i : per_frame_data)
	{
		VkSemaphoreCreateInfo semaphoreInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};

		VK_ENSURE(vkCreateSemaphore(context->get_context()->logical_device, &semaphoreInfo, vulkan_common::allocation_callback, &i.wait_render_finished_semaphore), "Failed to create render finnished semaphore")

			VkFenceCreateInfo fenceInfo {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};

		VK_ENSURE(vkCreateFence(context->get_context()->logical_device, &fenceInfo, vulkan_common::allocation_callback, &i.queue_submit_fence), "Failed to create fence");
	}


	present_command_buffers.resize(context->get_image_count());
	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = context->get_command_pool();
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = present_command_buffers.size();
	VK_ENSURE(vkAllocateCommandBuffers(context->get_context()->logical_device, &alloc_info, present_command_buffers.data()), "Failed to allocate command buffer");
}

void Framegraph::render_pass(const DrawInfo& draw_info, std::shared_ptr<FramegraphPass> pass)
{
	for (auto& child : pass->children_pass_list) {
		job_system::new_job([&, draw_info, child]
			{
				logger_warning("-> begin child pass %s", child->pass_name.c_str());
				render_pass(draw_info, child);
				logger_warning("-> end child pass %s", child->pass_name.c_str());
			});
	}
	logger_validate("wait children %s", pass->pass_name.c_str());
	job_system::wait_children();




	
	pass->render(draw_info);
	logger_error("end pass %s", pass->pass_name.c_str());
}

void Framegraph::render()
{
	logger_log("#### BEGIN FRAME");
	// Begin frame rendering
	// Acquire next image to draw on
	DrawInfo draw_info = swapchain->acquire_next_image();

	logger_log("#### ACQUIRED NEXT IMAGE");
	PerFrameData& current_frame_data = per_frame_data[draw_info.image_index];

	// Build command buffers
	for (auto& pass : graph_top)
	{
		job_system::new_job([&, draw_info] { render_pass(draw_info, pass); });		
	}
	job_system::wait_children();
	
	// Submit command buffers
	logger_log("### BEGIN SUBMIT PASS");


	std::vector<VkCommandBuffer> command_buffers;
	for (auto& pass : graph_top)
	{
		auto command_buffers = pass->collect_command_buffers(draw_info);
		command_buffers.insert(command_buffers.end(), command_buffers.begin(), command_buffers.end());
	}

	vkResetFences(context->get_context()->logical_device, 1, &current_frame_data.queue_submit_fence);
	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	
	VkSubmitInfo submit_infos{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &swapchain->image_acquire_semaphore[draw_info.frame_id],
		.pWaitDstStageMask = &wait_stage,
		.commandBufferCount = static_cast<uint32_t>(command_buffers.size()),
		.pCommandBuffers = command_buffers.data(),
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &current_frame_data.wait_render_finished_semaphore
	};
	context->get_context()->submit_graphic_queue(submit_infos, VK_NULL_HANDLE);


	logger_validate("SUCCESSFULLY validated submit");



	
	// Submit image
	logger_log("#### SUBMIT TO SWAPCHAIN");
	swapchain->submit_next_image(draw_info.image_index, { current_frame_data.wait_render_finished_semaphore });
	
	// End frame rendering
}

