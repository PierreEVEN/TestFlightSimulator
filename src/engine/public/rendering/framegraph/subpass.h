#pragma once


#include <array>
#include <memory>
#include <string>

#include "rendering/framegraph/framebuffer.h"

class FramegraphPass;
class Framegraph;
class Window;


class FramegraphSubpass
{
	friend FramegraphPass;
public:

	FramegraphSubpass(const std::string& name, const FramebufferDescription& in_framebuffer_description);

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