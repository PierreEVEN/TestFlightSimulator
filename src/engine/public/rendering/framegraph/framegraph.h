#pragma once
#include <array>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

/*
 * Pour build un proxy, chaque component doit connaitre ses passes de rendu
 * 
 * Au moment d'update les proxy, on collecte les components et on update les descriptors à ce moment là
 *
 * 
 */


class Window;

class Framebuffer
{
public:
	VkFormat image_format;
	VkFramebuffer buffer = VK_NULL_HANDLE;
	bool is_depth_stencil_buffer;
};


class FramegraphSubpass
{
	// var render_buffer;
	// var render_pass;

	// var scene_proxy;
	// var camera;
public:
	void render();
private:
	
	void create_subpass();


	
	std::string task_name;
	
	std::array<VkClearValue, 2> clear_values{
		VkClearValue{.color = { 0.6f, 0.9f, 1.f, 1.0f }},
		VkClearValue{.depthStencil = { 1.0f, 0 }}
	};

	Framebuffer framebuffer;
	
	uint32_t width = 1;
	uint32_t height = 1;
	VkSampleCountFlagBits pass_samples = VK_SAMPLE_COUNT_1_BIT;
	
};


/**
 * Pour chaque pass on va avoir besoin azdfza
 */


class FramegraphPass
{
public:
	void render();

private:


	void create_or_recreate_render_pass();

	VkRenderPass render_pass = VK_NULL_HANDLE;

	Window* context = nullptr;
	
	std::vector<FramegraphSubpass> group_tasks;	
	std::string group_name;
};



class Framegraph
{
public:
	void render();
private:	
	std::vector<FramegraphPass> graph_groups;		
};

