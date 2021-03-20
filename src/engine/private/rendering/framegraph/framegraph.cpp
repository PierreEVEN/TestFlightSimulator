

#include "rendering/framegraph/framegraph.h"


#include <vulkan/vulkan_core.h>

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
}

void Framegraph::render_pass(const DrawInfo& draw_info, std::shared_ptr<FramegraphPass> pass)
{
	for (auto& child : pass->children_pass_list) {
		// Render each subpass in a different job
		job_system::new_job([&, draw_info, child]
			{
				render_pass(draw_info, child);
			});
	}
	pass->render(draw_info);
}

void Framegraph::render()
{
	logger_log("#### BEGIN FRAME");
	// Begin frame rendering
	// Acquire next image to draw on
	DrawInfo draw_info = swapchain->acquire_next_image();

	logger_log("#### ACQUIRED NEXT IMAGE");
	
	// Build and submit render passes in different jobs
	for (auto& pass : graph_top)
	{
		job_system::new_job([&, draw_info] { render_pass(draw_info, pass); });		
	}

	logger_log("#### SUBMIT TO SWAPCHAIN");
	// Submit image
	swapchain->submit_next_image(draw_info.image_index, {});
	
	// End frame rendering
}

