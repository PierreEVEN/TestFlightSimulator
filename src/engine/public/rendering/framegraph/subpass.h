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

	FramegraphSubpass(const std::string& name, const FramebufferDescription& in_framebuffer_description, const VkClearValue& in_clear_value);

private:

	void build_buffer(FramegraphPass* parent);

	VkAttachmentDescription get_attachment_description();
	VkClearValue clear_value;

	std::string subpass_name;

	std::array<VkClearValue, 2> clear_values{
		VkClearValue{.color = { 1, 0, 0, 0 }},
	};

	std::shared_ptr<Framebuffer> framebuffer;
	FramebufferDescription framebuffer_description;
};