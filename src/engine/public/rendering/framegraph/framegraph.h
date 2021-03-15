#pragma once
#include <array>
#include <memory>
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


class FramegraphPass;
class Framegraph;
class Window;

struct FramebufferDescription
{
	VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;	
};

class Framebuffer
{
public:
	Framebuffer(Window* context, const FramebufferDescription& framebuffer_description, VkExtent2D extend);
	
	VkFormat image_format;
	VkImageUsageFlags image_usage;
	bool is_depth_stencil_buffer;
	VkSampleCountFlagBits samples;

	VkImage buffer_image = VK_NULL_HANDLE;
	VkDeviceMemory buffer_memory = VK_NULL_HANDLE;
	VkImageView buffer_view = VK_NULL_HANDLE;
};


class FramegraphSubpass
{
	friend FramegraphPass;
public:

	FramegraphSubpass(const std::string& name, const FramebufferDescription& in_framebuffer_description);
	
	void render();
private:

	void build_buffer(FramegraphPass* parent);
	
	VkAttachmentDescription get_attachment_description();


	
	std::string subpass_name;
	
	std::array<VkClearValue, 2> clear_values{
		VkClearValue{.color = { 0.6f, 0.9f, 1.f, 1.0f }},
		VkClearValue{.depthStencil = { 1.0f, 0 }}
	};

	std::shared_ptr<Framebuffer> framebuffer;
	FramebufferDescription framebuffer_description;
	
};

class FramegraphPass
{
	friend Framegraph;
	friend FramegraphSubpass;
public:

	FramegraphPass(VkExtent2D resolution, const std::string& in_pass_name, std::vector<FramegraphSubpass> in_subpasses);
	void render();
	
private:

	void init(Framegraph* in_parent);

	void create_or_recreate_render_pass();

	VkRenderPass render_pass = VK_NULL_HANDLE;
	
	std::vector<FramegraphSubpass> subpasses;	
	std::string pass_name;
	
	VkFramebuffer pass_framebuffer = VK_NULL_HANDLE;
	VkSampler color_sampler = VK_NULL_HANDLE;
	Framegraph* parent;

	VkExtent2D size;
	
};



class Framegraph
{
	friend FramegraphPass;
	friend FramegraphSubpass;
public:
	Framegraph(Window* in_context, const std::vector<FramegraphPass>& in_render_pass);
	void render();
private:	
	std::vector<FramegraphPass> graph_passes;

	Window* context;
};

