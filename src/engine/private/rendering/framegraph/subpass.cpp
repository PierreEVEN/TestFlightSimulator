
#include "rendering/framegraph/subpass.h"

#include "rendering/framegraph/framegraph.h"
#include "rendering/framegraph/renderpass.h"

FramegraphSubpass::FramegraphSubpass(const std::string& name, const FramebufferDescription& in_framebuffer_description)
	: framebuffer_description(in_framebuffer_description) {}

void FramegraphSubpass::build_buffer(FramegraphPass* parent)
{
	framebuffer = std::make_shared<Framebuffer>(parent->parent->context, framebuffer_description, parent->size);
}

VkAttachmentDescription FramegraphSubpass::get_attachment_description()
{
	VkAttachmentDescription subpass_attachment{};
	subpass_attachment.format = framebuffer->image_format;
	subpass_attachment.samples = framebuffer->samples;
	subpass_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	subpass_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	subpass_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	subpass_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	subpass_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	subpass_attachment.finalLayout = framebuffer->is_depth_stencil_buffer ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	return subpass_attachment;
}
