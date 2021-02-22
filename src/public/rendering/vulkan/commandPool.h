#pragma once

#include <thread>


#include "common.h"
#include "ios/logger.h"
#include "rendering/vulkan/utils.h"
#include "engine/jobSystem/worker.h"


namespace command_pool
{
	class CommandPool final
	{
	public:
		CommandPool(const std::thread::id thread_id, VkDevice logical_device, uint32_t queue)
			: pool_thread_id(thread_id) {
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = queue;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional
			VK_ENSURE(vkCreateCommandPool(logical_device, &poolInfo, vulkan_common::allocation_callback, &commandPool), "Failed to create command pool");
		}

		~CommandPool() {
			vkDestroyCommandPool(logical_device, commandPool, vulkan_common::allocation_callback);
		}

		[[nodiscard]] VkCommandPool& get() { return commandPool; }
		
		operator bool() const { return std::this_thread::get_id() == pool_thread_id; }

	private:

		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkDevice logical_device;
		std::thread::id pool_thread_id;
	};
	
	inline CommandPool* command_pools;

	inline void create_command_pools(VkDevice logical_device, uint32_t queue) {
		/*
		command_pools = (CommandPool*)malloc(job_system::G_CPU_THREAD_COUNT * sizeof(CommandPool));
		for (uint32_t i = 0; i < job_system::G_CPU_THREAD_COUNT; ++i) {
			new (command_pools + i) CommandPool(job_system::Workers::workers[i].get_thread_id(), logical_device, queue);
		}
		*/
	}

	inline void cleanup_command_pools() {
		/*
		for (uint32_t i = 0; i < job_system::G_CPU_THREAD_COUNT; ++i) {
			delete &command_pools[i];
		}
		*/
	}
	
	inline VkCommandPool& get() {
		/*
		for (uint32_t i = 0; i < job_system::G_CPU_THREAD_COUNT; ++i) {
			if (command_pools[i]) return command_pools[i].get();
		}
		*/
		logger::fail("Failed to find command pool on current thread");
	}
}
