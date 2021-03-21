#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "subpass.h"
#include "rendering/vulkan/swapchain.h"

class Framegraph;


struct PerImageData
{
	VkCommandPool command_pool = VK_NULL_HANDLE;
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;
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

	std::vector<std::string> dependencies_names;
	std::unordered_map<std::string, std::shared_ptr<FramegraphPass>> children_pass;
	std::vector<std::shared_ptr<FramegraphPass>> children_pass_list;
	std::shared_ptr<FramegraphPass> parent_pass = nullptr;

	[[nodiscard]] std::vector<VkCommandBuffer> collect_command_buffers(const DrawInfo& draw_info) const { return { per_image_data[draw_info.image_index].command_buffer }; }
};