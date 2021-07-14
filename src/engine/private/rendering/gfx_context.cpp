
#include "rendering/gfx_context.h"

#include "rendering/vulkan/common.h"
#include "rendering/vulkan/utils.h"
#include <config.h>
#include <set>
#include <cpputils/logger.hpp>

GfxContext::GfxContext(VkSurfaceKHR surface)
{
    select_physical_device(surface);
    create_logical_device(surface, physical_device);
    create_vma_allocator();
}

GfxContext::~GfxContext()
{
    LOG_INFO("destroy window window");

    destroy_vma_allocators();
    destroy_logical_device();
}



void GfxContext::submit_graphic_queue(const VkSubmitInfo& submit_infos, VkFence submit_fence)
{
    std::lock_guard<std::mutex> lock(queue_access_lock);
    VK_ENSURE(vkQueueSubmit(graphic_queue, 1, &submit_infos, submit_fence), "Failed to submit graphic queue");
}

VkResult GfxContext::submit_present_queue(const VkPresentInfoKHR& present_infos)
{
    std::lock_guard<std::mutex> lock(queue_access_lock);
    return vkQueuePresentKHR(present_queue, &present_infos);
}

void GfxContext::wait_device()
{
    std::lock_guard<std::mutex> queue_lock(queue_access_lock);
    vkDeviceWaitIdle(logical_device);
}

void GfxContext::select_physical_device(VkSurfaceKHR surface)
{
    // Get devices
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(vulkan_common::instance, &device_count, nullptr);
    if (device_count == 0) LOG_FATAL("No graphical device found.");
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(vulkan_common::instance, &device_count, devices.data());

    // Enumerate devices
    std::string PhysLog = "Found " + std::to_string(device_count) + " graphical devices : \n";
    for (const VkPhysicalDevice& device : devices)
    {
        VkPhysicalDeviceProperties pProperties;
        vkGetPhysicalDeviceProperties(device, &pProperties);
        PhysLog += "\t-" + std::string(pProperties.deviceName) + " (driver version : " + std::to_string(pProperties.driverVersion) + ")\n";
    }
    LOG_INFO("%s", PhysLog.c_str());

    // Pick desired device
    for (const auto& device : devices)
    {
        if (vulkan_utils::is_physical_device_suitable(surface, device))
        {
            VkPhysicalDeviceProperties pProperties;
            vkGetPhysicalDeviceProperties(device, &pProperties);
            // if (pProperties.deviceName[0] == 'G') continue;

            physical_device = device;
            break;
        }
    }

    VK_CHECK(physical_device, "Cannot find any suitable GPU");

    VkPhysicalDeviceProperties selected_device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &selected_device_properties);
    LOG_INFO("Picking physical device %d (%s)", selected_device_properties.deviceID, selected_device_properties.deviceName);
}

void GfxContext::create_logical_device(VkSurfaceKHR surface, VkPhysicalDevice device)
{
    LOG_INFO("Create logical device");

    queue_families = vulkan_utils::find_device_queue_families(surface, device);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t>                   unique_queue_families = {queue_families.graphic_family.value(), queue_families.present_family.value()};
    float                                queue_priorities      = 1.0f;
    for (uint32_t queueFamily : unique_queue_families)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queue_priorities;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE; // Sample Shading
    deviceFeatures.fillModeNonSolid  = VK_TRUE; // Wireframe
    deviceFeatures.geometryShader    = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos       = queueCreateInfos.data();
    createInfo.pEnabledFeatures        = &deviceFeatures;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(config::required_device_extensions.size());
    createInfo.ppEnabledExtensionNames = config::required_device_extensions.data();

    if (config::use_validation_layers)
    {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(config::required_validation_layers.size());
        createInfo.ppEnabledLayerNames = config::required_validation_layers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    VK_ENSURE(vkCreateDevice(physical_device, &createInfo, vulkan_common::allocation_callback, &logical_device), "Failed to create logical device");

    if (queue_families.transfert_family.has_value()) vkGetDeviceQueue(logical_device, queue_families.transfert_family.value(), 0, &transfert_queue);
    vkGetDeviceQueue(logical_device, queue_families.graphic_family.value(), 0, &graphic_queue);
    vkGetDeviceQueue(logical_device, queue_families.present_family.value(), 0, &present_queue);

    VK_CHECK(transfert_queue, "VkLogicalDevice is null");
    VK_CHECK(graphic_queue, "Failed to find graphic queue");
    VK_CHECK(present_queue, "Failed to find present queue");
}

void GfxContext::create_vma_allocator()
{
    LOG_INFO("Create memory allocators");
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice         = physical_device;
    allocatorInfo.device                 = logical_device;
    allocatorInfo.instance               = vulkan_common::instance;

    vmaCreateAllocator(&allocatorInfo, &vulkan_memory_allocator);
}

void GfxContext::destroy_vma_allocators()
{
    LOG_INFO("Destroy memory allocators");
    vmaDestroyAllocator(vulkan_memory_allocator);
}

void GfxContext::destroy_logical_device()
{
    LOG_INFO("Destroy logical device");
    vkDestroyDevice(logical_device, vulkan_common::allocation_callback);
}

static std::shared_ptr<GfxContext> gfx_context_reference;

GfxContext* GfxContext::get_internal()
{
    if (!gfx_context_reference)
    {
        LOG_WARNING("trying to get gfx context but gfx context has not been created");
        return nullptr;
    }
    return gfx_context_reference.get();
}

void GfxContext::set_internal(std::shared_ptr<GfxContext> gfx_context)
{
    gfx_context_reference = gfx_context;
}
