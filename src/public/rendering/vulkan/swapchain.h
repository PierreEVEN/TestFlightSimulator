#pragma once

#include "rendering/vulkan/utils.h"

class Window;

class Swapchain
{
public:
	explicit Swapchain(const VkExtent2D& extend, Window* window);
	virtual ~Swapchain();
	void resize_swapchain(VkExtent2D extend, bool force_rebuild = false, bool safe_resize = true);
	
private:

	void create_or_recreate();
	void destroy();

	VkViewport viewport;
	VkExtent2D swapchain_extend;
	VkRect2D scissor;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	Window* surface_window = nullptr;
	
};
