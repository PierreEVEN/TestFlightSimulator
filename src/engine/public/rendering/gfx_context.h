#pragma once
#include "vulkan/utils.h"

#include <mutex>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

class GfxContext
{
  public:
    GfxContext(VkSurfaceKHR surface);
    ~GfxContext();

    VkPhysicalDevice                 physical_device = VK_NULL_HANDLE;
    VkDevice                         logical_device  = VK_NULL_HANDLE;
    vulkan_utils::QueueFamilyIndices queue_families;

    VmaAllocator vulkan_memory_allocator = VK_NULL_HANDLE;

    void     submit_graphic_queue(const VkSubmitInfo& submit_infos, VkFence submit_fence);
    VkResult submit_present_queue(const VkPresentInfoKHR& present_infos);
    void     wait_device();

  private:
    std::mutex queue_access_lock;
    VkQueue    graphic_queue   = VK_NULL_HANDLE;
    VkQueue    transfert_queue = VK_NULL_HANDLE;
    VkQueue    present_queue   = VK_NULL_HANDLE;

    void select_physical_device(VkSurfaceKHR surface);
    void create_logical_device(VkSurfaceKHR surface);
    void create_vma_allocator();

    void destroy_vma_allocators();
    void destroy_logical_device();
};
