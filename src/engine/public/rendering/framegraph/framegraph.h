#pragma once
#include <vector>

#include "framebuffer.h"
#include "rendering/vulkan/swapchain.h"


class Swapchain;
class FramegraphSubpass;
class FramegraphPass;

class Framegraph
{
	friend FramegraphPass;
	friend FramegraphSubpass;
public:
	Framegraph(Window* in_context, const std::vector<FramegraphPass>& in_render_pass);
	void render();
private:
	void render_pass(const DrawInfo& draw_info, FramegraphPass* pass);
	std::vector<FramegraphPass> graph_passes;
	std::vector<FramegraphPass*> graph_top;

	Swapchain* swapchain = nullptr;

	Window* context;
};

