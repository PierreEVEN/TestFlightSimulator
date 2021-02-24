
#include "rendering/window.h"


#include <mutex>
#include <set>
#include <unordered_map>


#include "config.h"
#include "ios/logger.h"

std::mutex window_map_lock;
std::unordered_map<GLFWwindow*, Window*> window_map;

void framebuffer_size_callback(GLFWwindow* handle, const int res_x, const int res_y) {
	auto window_ptr = window_map.find(handle);
	if (window_ptr != window_map.end())	{
		window_ptr->second->resize_window(res_x, res_y);
	}
}

Window::Window(const int res_x, const int res_y, const char* name, bool fullscreen)
	: window_width(res_x), window_height(res_y), window_name(name)
{
	window_map_lock.lock();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	if (fullscreen) glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	// Create window handle
	window_handle = glfwCreateWindow(res_x, res_y, name, nullptr, nullptr);
	if (!window_handle) logger::fail("failed to create glfw Window");
	glfwSetFramebufferSizeCallback(window_handle, framebuffer_size_callback);

	// Create window surface
	create_window_surface();

	// Get vulkan context from existing window or create a new one
	if (!window_map.empty()) initialize_from_window(*window_map.begin()->second);
	else create_vulkan_context();
	
	window_map[window_handle] = this;
	
	logger::validate("created Window '%s' ( %d x %d )", name, res_x, res_y);
	window_map_lock.unlock();
}

Window::~Window() {
	window_map_lock.lock();
	window_map.erase(window_map.find(window_handle));
	glfwDestroyWindow(window_handle);
	window_map_lock.unlock();
}

void Window::resize_window(const int res_x, const int res_y) {
	window_width = res_x;
	window_height = res_y;
}

bool Window::begin_frame()
{
	glfwPollEvents();
	return !glfwWindowShouldClose(window_handle);
}

bool Window::end_frame()
{
	glfwSwapBuffers(window_handle);
	return !glfwWindowShouldClose(window_handle);
}

void Window::initialize_from_window(const Window& other)
{
	logger::log("use context from '%s' window", other.window_name);
	physical_device = other.physical_device;
	logical_device = other.logical_device;
	graphic_queue = other.graphic_queue;
	transfert_queue = other.transfert_queue;
	present_queue = other.present_queue;
	vulkan_memory_allocator = other.vulkan_memory_allocator;
}

void Window::create_vulkan_context()
{
	logger::log("create new vulkan context");
	select_physical_device();
	create_logical_device();
	create_vma_allocator();
}

void Window::create_window_surface()
{
	VK_ENSURE(glfwCreateWindowSurface(vulkan_common::instance, window_handle, nullptr, &surface) != VK_SUCCESS, "Failed to create Window surface");
	VK_CHECK(surface, "VkSurfaceKHR is null");
	logger::log("Create Window surface");
}

void Window::select_physical_device()
{
	// Get devices
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(vulkan_common::instance, &device_count, nullptr);
	if (device_count == 0) logger::fail("No graphical device found.");
	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(vulkan_common::instance, &device_count, devices.data());

	// Enumerate devices
	std::string PhysLog = "Found " + std::to_string(device_count) + " graphical devices : \n";
	for (const VkPhysicalDevice& device : devices) {
		VkPhysicalDeviceProperties pProperties;
		vkGetPhysicalDeviceProperties(device, &pProperties);
		PhysLog += "\t-" + std::string(pProperties.deviceName) + " (driver version : " + std::to_string(pProperties.driverVersion) + ")\n";
	}
	logger::log(PhysLog.c_str());

	// Pick desired device
	for (const auto& device : devices) {
		if (vulkan_utils::is_physical_device_suitable(surface, device)) {
			physical_device = device;
			break;
		}
	}

	VK_CHECK(physical_device, "Cannot find any suitable GPU");

	VkPhysicalDeviceProperties selected_device_properties;
	vkGetPhysicalDeviceProperties(physical_device, &selected_device_properties);
	logger::log("Picking physical device %d (%s)", selected_device_properties.deviceID, selected_device_properties.deviceName);
}

void Window::create_logical_device()
{
	logger::log("Create logical device");

	vulkan_utils::queue_family_indices queue_families = vulkan_utils::find_device_queue_families(surface, physical_device);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { queue_families.graphic_family.value(), queue_families.present_family.value() };
	float queuePriority = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE; // Sample Shading
	deviceFeatures.fillModeNonSolid = VK_TRUE; // Wireframe

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(config::required_device_extensions.size());
	createInfo.ppEnabledExtensionNames = config::required_device_extensions.data();

	if (config::use_validation_layers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(config::required_validation_layers.size());
		createInfo.ppEnabledLayerNames = config::required_validation_layers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}
	
	VK_ENSURE(vkCreateDevice(physical_device, &createInfo, vulkan_common::allocation_callback, &logical_device), "Failed to create logical device");

	if (queue_families.transfert_family.has_value())	vkGetDeviceQueue(logical_device, queue_families.transfert_family.value(), 0, &transfert_queue);
	vkGetDeviceQueue(logical_device, queue_families.graphic_family.value(), 0, &graphic_queue);
	vkGetDeviceQueue(logical_device, queue_families.present_family.value(), 0, &present_queue);

	VK_CHECK(transfert_queue, "VkLogicalDevice is null");
	VK_CHECK(graphic_queue, "Failed to find graphic queue");
	VK_CHECK(present_queue, "Failed to find present queue");
}

void Window::create_vma_allocator()
{
	logger::log("Create memory allocators");
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = physical_device;
	allocatorInfo.device = logical_device;
	allocatorInfo.instance = vulkan_common::instance;

	vmaCreateAllocator(&allocatorInfo, &vulkan_memory_allocator);
}
