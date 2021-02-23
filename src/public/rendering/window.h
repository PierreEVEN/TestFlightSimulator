#pragma once

#include <memory>


#include "vulkan/commandPool.h"
#include "vulkan/common.h"

class WindowContext
{
public:
	WindowContext(GLFWwindow* handle, VkSurfaceKHR surface);
	~WindowContext();
	
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice logical_device = VK_NULL_HANDLE;
	vulkan_utils::queue_family_indices queue_families;
	VkQueue graphic_queue = VK_NULL_HANDLE;
	VkQueue transfert_queue = VK_NULL_HANDLE;
	VkQueue present_queue = VK_NULL_HANDLE;
	VmaAllocator vulkan_memory_allocator = VK_NULL_HANDLE;
	command_pool::Container* command_pool;

private:

	void select_physical_device(VkSurfaceKHR surface);
	void create_logical_device(VkSurfaceKHR surface);
	void create_vma_allocator();
};


class Window
{
public:
	Window(int res_x, int res_y, const char* name, bool fullscreen = false);
	virtual ~Window();
	
	bool begin_frame();	
	bool end_frame();

private:

	std::shared_ptr<WindowContext> context;
	
	GLFWwindow* window_handle;
	
	
	int window_width;
	int window_height;
	const char* window_name;
	VkSurfaceKHR surface = VK_NULL_HANDLE;

	friend void framebuffer_size_callback(GLFWwindow* handle, int res_x, int res_y);
	void resize_window(int res_x, int res_y);
		
	void create_window_surface();
};
