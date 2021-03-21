#pragma once
#include <unordered_map>
#include <vector>

#include "framebuffer.h"
#include "rendering/vulkan/swapchain.h"


class Swapchain;
class FramegraphSubpass;
class FramegraphPass;

struct PerFrameData
{
	VkSemaphore wait_render_finished_semaphore = VK_NULL_HANDLE;
	VkFence queue_submit_fence = VK_NULL_HANDLE;
};

class Framegraph
{
	friend FramegraphPass;
	friend FramegraphSubpass;
public:
	Framegraph(Window* in_context, const std::vector<std::shared_ptr<FramegraphPass>>& in_render_pass);
	void render();
private:
	void render_pass(const DrawInfo& draw_info, std::shared_ptr<FramegraphPass> pass);
	std::unordered_map<std::string, std::shared_ptr<FramegraphPass>> graph_passes;
	std::vector<std::shared_ptr<FramegraphPass>> graph_top;
	
	std::vector<PerFrameData> per_frame_data;
	std::vector<VkCommandBuffer> present_command_buffers;

	Swapchain* swapchain = nullptr;
	Window* context = nullptr;
};

