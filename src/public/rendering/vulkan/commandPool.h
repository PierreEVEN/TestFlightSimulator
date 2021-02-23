#pragma once

#include <thread>


#include "common.h"
#include "rendering/vulkan/utils.h"


namespace command_pool
{
	class CommandPool final
	{
	public:
		CommandPool(VkDevice logical_device, uint32_t queue);
		void destroy();

		[[nodiscard]] VkCommandPool& get();

		explicit operator bool();

	private:
		bool is_created = false;
		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkDevice pool_logical_device;
		std::thread::id pool_thread_id;
	};

	class Container final {
	public:
		Container(VkDevice logical_device, uint32_t queue);
		~Container();

		[[nodiscard]] VkCommandPool& get();

	private:

		CommandPool* command_pools = nullptr;
		size_t command_pool_count = 0;
		
		const VkDevice context_logical_device;
		const uint32_t context_queue;
	};

}
