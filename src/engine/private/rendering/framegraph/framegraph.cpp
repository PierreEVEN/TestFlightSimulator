

#include "rendering/framegraph/framegraph.h"


#include <vulkan/vulkan_core.h>



#include "jobSystem/job_system.h"
#include "rendering/window.h"
#include "rendering/vulkan/commandPool.h"

Framebuffer::Framebuffer(Window* context, const FramebufferDescription& framebuffer_description, VkExtent2D extend)
	:
	samples(framebuffer_description.samples),
	image_usage(framebuffer_description.usage),
	image_format(framebuffer_description.format)
{
	is_depth_stencil_buffer = image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	
	VkImageAspectFlags aspectMask = 0;

	if (is_depth_stencil_buffer)
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else
	{
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkImageCreateInfo image{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = image_format,
		.extent = {
			.width = extend.width,
			.height = extend.height,
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = samples,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = image_usage | VK_IMAGE_USAGE_SAMPLED_BIT
	};

	VkMemoryRequirements memReqs;
	VK_ENSURE(vkCreateImage(context->get_context()->logical_device, &image, vulkan_common::allocation_callback, &buffer_image), "failed to create image");
	vkGetImageMemoryRequirements(context->get_context()->logical_device, buffer_image, &memReqs);

	VkMemoryAllocateInfo memAlloc = VkMemoryAllocateInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memReqs.size,
		.memoryTypeIndex = vulkan_utils::find_memory_type(context->get_context()->physical_device, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	VK_ENSURE(vkAllocateMemory(context->get_context()->logical_device, &memAlloc, vulkan_common::allocation_callback, &buffer_memory), "failed to allocate image memory");
	VK_ENSURE(vkBindImageMemory(context->get_context()->logical_device, buffer_image, buffer_memory, 0), "failed to bind image memory");

	VkImageViewCreateInfo view_info;
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = image_format;
	view_info.subresourceRange.aspectMask = aspectMask;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	view_info.image = buffer_image;
	view_info.flags = 0;
	view_info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view_info.pNext = nullptr;
	VK_ENSURE(vkCreateImageView(context->get_context()->logical_device, &view_info, vulkan_common::allocation_callback, &buffer_view), "failed to create image view");
}

FramegraphSubpass::FramegraphSubpass(const std::string& name, const FramebufferDescription& in_framebuffer_description)
	: framebuffer_description(in_framebuffer_description) {}

void FramegraphSubpass::render()
{
}

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

FramegraphPass::FramegraphPass(VkExtent2D resolution, const std::string& in_pass_name, std::vector<FramegraphSubpass> in_subpasses)
	: subpasses(in_subpasses), pass_name(in_pass_name), size(resolution)
{
	
}

void FramegraphPass::render()
{
	for (auto& task : subpasses)
	{
		job_system::new_job([&task] {
			task.render();
		});
	}
	job_system::wait_children();
}

void FramegraphPass::init(Framegraph* in_parent)
{
	parent = in_parent;

	create_or_recreate_render_pass();	
}


void FramegraphPass::create_or_recreate_render_pass()
{
	for (auto& subpass : subpasses)
	{
		subpass.build_buffer(this);
	}
	
	logger_log("create render pass group '%s'", pass_name.c_str());	
	if (render_pass != VK_NULL_HANDLE) vkDestroyRenderPass(parent->context->get_context()->logical_device, render_pass, vulkan_common::allocation_callback);

	/*
	 *  QUERY SUBPASSES
	 */
	
	std::vector<VkAttachmentDescription> attachment_descriptions;
	std::vector<VkAttachmentReference> color_attachment_references;
	std::optional<VkAttachmentReference> depth_stencil_attachment_reference;
	
	for (int i = 0; i < subpasses.size(); ++i)
	{
		attachment_descriptions.push_back(subpasses[i].get_attachment_description());
		if (subpasses[i].framebuffer->is_depth_stencil_buffer)
		{
			if (depth_stencil_attachment_reference) logger_error("cannot handle multiple depth attachment per render pass");
			depth_stencil_attachment_reference = VkAttachmentReference{
				.attachment = static_cast<uint32_t>(i),
				.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				};
		}
		else {
			color_attachment_references.push_back(VkAttachmentReference{
				.attachment = static_cast<uint32_t>(i),
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				});
		}
	}

	VkSubpassDescription subpass_description = {};
	subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_description.pColorAttachments = color_attachment_references.data();
	subpass_description.colorAttachmentCount = static_cast<uint32_t>(color_attachment_references.size());
	subpass_description.pDepthStencilAttachment = depth_stencil_attachment_reference.has_value() ? &depth_stencil_attachment_reference.value() : nullptr;

	/*
	 *  SETUP SUBPASSES DEPENDENCIES
	 */
	
	// Use subpass dependencies for attachment layout transitions
	std::array<VkSubpassDependency, 2> dependencies = {};

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attachment_descriptions.data();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachment_descriptions.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass_description;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies.data();
	VK_ENSURE(vkCreateRenderPass(parent->context->get_context()->logical_device, &renderPassInfo, vulkan_common::allocation_callback, &render_pass), "failed to create render pass");

	std::vector<VkImageView> render_pass_attachments;

	for (const auto& pass : subpasses)
	{
		render_pass_attachments.push_back(pass.framebuffer->buffer_view);
	}

	VkFramebufferCreateInfo fbufCreateInfo = {};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.pNext = NULL;
	fbufCreateInfo.renderPass = render_pass;
	fbufCreateInfo.pAttachments = render_pass_attachments.data();
	fbufCreateInfo.attachmentCount = static_cast<uint32_t>(render_pass_attachments.size());
	fbufCreateInfo.width = size.width;
	fbufCreateInfo.height = size.height;
	fbufCreateInfo.layers = 1;
	fbufCreateInfo.pNext = nullptr;
	VK_ENSURE(vkCreateFramebuffer(parent->context->get_context()->logical_device, &fbufCreateInfo, nullptr, &pass_framebuffer));

	// Create sampler to sample from the color attachments
	VkSamplerCreateInfo sampler;
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.magFilter = VK_FILTER_NEAREST;
	sampler.minFilter = VK_FILTER_NEAREST;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	sampler.flags = 0;
	sampler.anisotropyEnable = VK_TRUE;
	sampler.unnormalizedCoordinates = VK_FALSE;
	sampler.compareEnable = VK_FALSE;
	sampler.pNext = nullptr;
	VK_ENSURE(vkCreateSampler(parent->context->get_context()->logical_device, &sampler, nullptr, &color_sampler));

	
}

Framegraph::Framegraph(Window* in_context, const std::vector<FramegraphPass>& in_render_pass)
	: context(in_context), graph_passes(in_render_pass)
{
	for (auto& render_pass : graph_passes)
	{
		render_pass.init(this);
	}
	
}

void Framegraph::render()
{
	for (auto& group : graph_passes)
	{
		group.render();
	}
}

