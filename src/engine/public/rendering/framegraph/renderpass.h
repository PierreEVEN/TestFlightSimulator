#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "subpass.h"
#include "rendering/vulkan/swapchain.h"

class Framegraph;

class FramegraphPass
{
	friend Framegraph;
	friend FramegraphSubpass;
	friend Swapchain;
public:

	FramegraphPass(VkExtent2D resolution, const std::string& in_pass_name, const std::vector<std::string>& in_dependencies, std::vector<FramegraphSubpass> in_subpasses);
	void render(DrawInfo draw_info);

private:

	void init(Framegraph* in_parent);

	void create_frame_objects();


	void create_render_pass();

	VkRenderPass render_pass = VK_NULL_HANDLE;

	std::vector<FramegraphSubpass> subpasses;
	std::string pass_name;

	VkFramebuffer pass_framebuffer = VK_NULL_HANDLE;
	VkSampler color_sampler = VK_NULL_HANDLE;
	Framegraph* parent;

	VkExtent2D size;

	std::vector<VkCommandBuffer> command_buffers;
	std::vector<VkSemaphore> image_acquire_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;

	std::vector<std::string> dependencies;
	std::vector<FramegraphPass*> children_pass;
	FramegraphPass* parent_pass = nullptr;

	
};