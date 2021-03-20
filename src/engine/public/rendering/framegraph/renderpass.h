#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "subpass.h"
#include "rendering/vulkan/swapchain.h"

class Framegraph;

struct PerFrameData
{
	VkSemaphore wait_render_finished_semaphore;
	VkFence queue_submit_fence;
};

struct PerImageData
{
	VkCommandBuffer command_buffer;
};

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
	std::vector<PerImageData> per_image_data;
	std::vector<PerFrameData> per_frame_data;

	std::vector<std::string> dependencies_names;
	std::unordered_map<std::string, std::shared_ptr<FramegraphPass>> children_pass;
	std::vector<std::shared_ptr<FramegraphPass>> children_pass_list;
	std::shared_ptr<FramegraphPass> parent_pass = nullptr;

	
};