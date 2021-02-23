#include "rendering/vulkan/commandPool.h"
#include "ios/logger.h"
#include "engine/jobSystem/worker.h"

namespace command_pool
{
	CommandPool::CommandPool(VkDevice logical_device, uint32_t queue)

		: pool_logical_device(logical_device) {
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queue;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional
		VK_ENSURE(vkCreateCommandPool(logical_device, &poolInfo, vulkan_common::allocation_callback, &commandPool), "Failed to create command pool");
	}

	CommandPool::~CommandPool()
	{
		if (is_created) vkDestroyCommandPool(pool_logical_device, commandPool, vulkan_common::allocation_callback);
	}

	VkCommandPool& CommandPool::get()
	{
		return commandPool;
	}

	CommandPool::operator bool()
	{
		if (!is_created)
		{
			is_created = true;
			pool_thread_id = std::this_thread::get_id();
			return true;
		}
		return std::this_thread::get_id() == pool_thread_id;
	}

	Container::Container(VkDevice logical_device, uint32_t queue)
		: context_logical_device(logical_device), context_queue(queue)
	{
		command_pool_count = job_system::Worker::get_worker_count();
		command_pools = static_cast<CommandPool*>(std::malloc(command_pool_count * sizeof(CommandPool)));
		for (int i = 0; i < job_system::Worker::get_worker_count(); ++i)
		{
			new (command_pools + i) CommandPool(logical_device, queue);
		}
	}

	VkCommandPool& Container::get()
	{
		for (int i = 0; i < command_pool_count; ++i)
		{
			if (CommandPool& pool = command_pools[i])
			{
				return pool.get();
			}
		}
		logger::fail("no command pool is available on current thread");
		exit(EXIT_FAILURE);
	}
}
