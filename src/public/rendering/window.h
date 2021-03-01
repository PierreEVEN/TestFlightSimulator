#pragma once

#include <memory>
#include <mutex>



#include "assets/GraphicResource.h"
#include "vulkan/commandPool.h"
#include "vulkan/common.h"

class ImGuiInstance;
class Framebuffer;
class Swapchain;

class WindowContext
{
public:
	WindowContext(GLFWwindow* handle, VkSurfaceKHR surface);
	~WindowContext();
	
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice logical_device = VK_NULL_HANDLE;
	vulkan_utils::QueueFamilyIndices queue_families;

	VmaAllocator vulkan_memory_allocator = VK_NULL_HANDLE;

	void submit_graphic_queue(const VkSubmitInfo& submit_infos, VkFence& submit_fence);
	VkResult submit_present_queue(const VkPresentInfoKHR& present_infos);
	void wait_device();
private:
	std::mutex queue_access_lock;
	VkQueue graphic_queue = VK_NULL_HANDLE;
	VkQueue transfert_queue = VK_NULL_HANDLE;
	VkQueue present_queue = VK_NULL_HANDLE;

	void select_physical_device(VkSurfaceKHR surface);
	void create_logical_device(VkSurfaceKHR surface);
	void create_vma_allocator();
	
	void destroy_vma_allocators();
	void destroy_logical_device();
};


class Window
{
public:
	Window(int res_x, int res_y, const char* name, bool fullscreen = false, bool img_context = false);
	virtual ~Window();
	
	bool begin_frame();	
	bool end_frame();


	[[nodiscard]] VkCommandPool get_command_pool() const { return command_pool->get(); }
	[[nodiscard]] GLFWwindow* get_handle() const { return window_handle; }
	[[nodiscard]] WindowContext* get_context() const { return context.get(); }
	[[nodiscard]] VkSurfaceKHR get_surface() const { return surface; }
	[[nodiscard]] uint32_t get_image_count() const { return swapchain_image_count; }
	[[nodiscard]] vulkan_utils::SwapchainSupportDetails get_support_details() const { return swapchain_support_details; }
	[[nodiscard]] VkSurfaceFormatKHR get_surface_format() const { return swapchain_surface_format;  }
	[[nodiscard]] VkPresentModeKHR get_present_mode() const { return swapchain_present_mode; }
	[[nodiscard]] VkRenderPass get_render_pass() const { return render_pass; }
	[[nodiscard]] GraphicResourceManager& get_resource_manager() { return resource_manager; }

	[[nodiscard]] uint32_t get_msaa_sample_count() const { return msaa_sample_count; }
	[[nodiscard]] uint32_t get_max_msaa_sample_count() const { return max_msaa_sample_count; }

private:

	bool has_imgui_context;
	std::shared_ptr<WindowContext> context;
	
	GLFWwindow* window_handle;
	command_pool::Container* command_pool;
	vulkan_utils::SwapchainSupportDetails	swapchain_support_details;
	VkSurfaceFormatKHR swapchain_surface_format;
	VkPresentModeKHR swapchain_present_mode;
	VkSampleCountFlagBits max_msaa_sample_count;
	VkSampleCountFlagBits msaa_sample_count;
	uint32_t swapchain_image_count;

	GraphicResourceManager resource_manager;
	ImGuiInstance* imgui_instance = nullptr;	

	std::vector<VkSemaphore> image_acquire_semaphore;
	std::vector<VkSemaphore> render_finished_semaphores;
	std::vector<VkFence> in_flight_fences;
	std::vector<VkFence> images_in_flight;
	size_t current_frame_id = 0;
	bool bHasViewportBeenResized = false;
	
	uint32_t window_width;
	uint32_t window_height;
	const char* window_name;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkRenderPass render_pass = VK_NULL_HANDLE;
	Framebuffer* back_buffer;
	std::vector<VkCommandBuffer> command_buffers;

	friend void framebuffer_size_callback(GLFWwindow* handle, int res_x, int res_y);
	void resize_window(int res_x, int res_y);	
	void create_window_surface();
	void setup_swapchain_property();
	void create_or_recreate_render_pass();
	void create_command_buffer();
	void create_fences_and_semaphores();

	void render();
	
	void destroy_fences_and_semaphores();
	void destroy_command_buffer();
	void destroy_render_pass();
	void destroy_window_surface();
};
