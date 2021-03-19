#pragma once

#include <vulkan/vulkan_core.h>

class Window;

struct FramebufferDescription
{
	VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
	bool is_depth_buffer = false;
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
