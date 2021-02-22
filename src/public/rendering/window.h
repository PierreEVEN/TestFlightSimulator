#pragma once

#include "vulkan/common.h"

class Window
{
public:
	Window(int res_x, int res_y, const char* name, bool fullscreen = false);
	virtual ~Window();
	
	bool begin_frame();	
	bool end_frame();

private:
	
	GLFWwindow* window_handle;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice logical_device = VK_NULL_HANDLE;
	VkQueue graphic_queue = VK_NULL_HANDLE;
	VkQueue transfert_queue = VK_NULL_HANDLE;
	VkQueue present_queue = VK_NULL_HANDLE;
	VmaAllocator vulkan_memory_allocator = VK_NULL_HANDLE;
	
	
	int window_width;
	int window_height;
	const char* window_name;

	friend void framebuffer_size_callback(GLFWwindow* handle, int res_x, int res_y);
	void resize_window(int res_x, int res_y);
	
	void initialize_from_window(const Window& other);
	void create_vulkan_context();
	
	void create_window_surface();
	void select_physical_device();
	void create_logical_device();
	void create_vma_allocator();
	
};
