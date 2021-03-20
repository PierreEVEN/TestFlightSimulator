#pragma once

#include "rendering/vulkan/utils.h"

class FramegraphPass;
class Window;

struct DrawInfo
{
	FramegraphPass* current_pass = nullptr;
	Window* context = nullptr;
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	uint32_t image_index = 0;
	uint32_t frame_id = 0;
};

class Swapchain
{
	friend FramegraphPass;
public:
	explicit Swapchain(const VkExtent2D& extend, Window* window);
	virtual ~Swapchain();
	void set_size(VkExtent2D extend, bool force_rebuild = false, bool safe_resize = true);

	[[nodiscard]] VkSwapchainKHR get() const { return swapchain; }


	DrawInfo acquire_next_image();
	void submit_next_image(uint32_t image_index, std::vector<VkSemaphore> render_finished_semaphores);

private:

	void create_fences_and_semaphores();
	
	void create_or_recreate();
	void destroy();


	uint32_t image_in_flight = 0;

	std::vector<VkSemaphore> image_acquire_semaphore;
	std::vector<VkFence> in_flight_fences;
	std::vector<VkFence> images_in_flight;
	
	VkViewport viewport;
	VkExtent2D swapchain_extend;
	VkRect2D scissor;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	Window* surface_window = nullptr;	
};
