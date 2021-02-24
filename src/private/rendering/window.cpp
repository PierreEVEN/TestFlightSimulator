
#include "rendering/window.h"


#include <mutex>
#include <set>
#include <unordered_map>
#include <array>

#include "config.h"
#include "ios/logger.h"
#include "rendering/vulkan/commandPool.h"
#include "rendering/vulkan/swapchain.h"

std::mutex window_map_lock;
std::unordered_map<GLFWwindow*, Window*> window_map;

void framebuffer_size_callback(GLFWwindow* handle, const int res_x, const int res_y) {
	auto window_ptr = window_map.find(handle);
	if (window_ptr != window_map.end())	{
		window_ptr->second->resize_window(res_x, res_y);
	}
}

WindowContext::WindowContext(GLFWwindow* handle, VkSurfaceKHR surface)
{
	select_physical_device(surface);
	create_logical_device(surface);
	create_vma_allocator();
}

WindowContext::~WindowContext()
{
	logger::log("destroy window context");

	destroy_vma_allocators();
	destroy_logical_device();
}

Window::Window(const int res_x, const int res_y, const char* name, bool fullscreen)
	: window_width(res_x), window_height(res_y), window_name(name)
{
	std::lock_guard<std::mutex> lock(window_map_lock);
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
	if (!window_map.empty()) {
		logger::log("use context from '%s' window", window_map.begin()->second->window_name);
		context = window_map.begin()->second->context;
	}
	else {
		logger::log("create new vulkan context");
		context = std::make_shared<WindowContext>(window_handle, surface);
	}

	// Create window vulkan objects
	command_pool = new command_pool::Container(context->logical_device, context->queue_families.graphic_family.value());
	setup_swapchain_property();
	create_or_recreate_render_pass();

	//swapchain = new Swapchain(VkExtent2D{ static_cast<uint32_t>(window_width), static_cast<uint32_t>(window_height) }, this);
	
	window_map[window_handle] = this;
	
	logger::validate("created Window '%s' ( %d x %d )", name, res_x, res_y);
}

Window::~Window() {
	std::lock_guard<std::mutex> lock(window_map_lock);
	window_map.erase(window_map.find(window_handle));
	glfwDestroyWindow(window_handle);

	//delete swapchain;
	destroy_render_pass();
	delete command_pool;
	context = nullptr;
	destroy_window_surface();
	logger::validate("successfully destroyed window");
}

void Window::setup_swapchain_property()
{
	swapchain_support_details = vulkan_utils::get_swapchain_support_details(surface, context->physical_device);
	swapchain_surface_format = vulkan_utils::choose_swapchain_surface_format(context->physical_device, surface);
	swapchain_present_mode = vulkan_utils::choose_swapchain_present_mode(swapchain_support_details.present_modes);
	swapchain_image_count = swapchain_support_details.capabilities.minImageCount + 1;
	max_msaa_sample_count = vulkan_utils::get_max_usable_sample_count(context->physical_device);
	msaa_sample_count = max_msaa_sample_count;
	logger::log("swapchain details : \n\
		\t-max samples : %d\n\
		\t-image count : %d\n\
		\t-present mode : %d\n\
		\t-surface format : %d\
		", max_msaa_sample_count, swapchain_image_count, swapchain_present_mode, swapchain_surface_format);
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

void Window::create_window_surface()
{
	VK_ENSURE(glfwCreateWindowSurface(vulkan_common::instance, window_handle, nullptr, &surface) != VK_SUCCESS, "Failed to create Window surface");
	VK_CHECK(surface, "VkSurfaceKHR is null");
	logger::log("Create Window surface");
}

void WindowContext::select_physical_device(VkSurfaceKHR surface)
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

void WindowContext::create_logical_device(VkSurfaceKHR surface)
{
	logger::log("Create logical device");

	queue_families = vulkan_utils::find_device_queue_families(surface, physical_device);

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

void WindowContext::create_vma_allocator()
{
	logger::log("Create memory allocators");
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = physical_device;
	allocatorInfo.device = logical_device;
	allocatorInfo.instance = vulkan_common::instance;

	vmaCreateAllocator(&allocatorInfo, &vulkan_memory_allocator);
}

void WindowContext::destroy_vma_allocators()
{
	logger::log("Destroy memory allocators");
	vmaDestroyAllocator(vulkan_memory_allocator);
}

void WindowContext::destroy_logical_device()
{
	logger::log("Destroy logical device");
	vkDestroyDevice(logical_device, vulkan_common::allocation_callback);
}

void Window::create_or_recreate_render_pass()
{
	if (render_pass != VK_NULL_HANDLE) {
		destroy_render_pass();
	}

	logger::log("Create render pass");
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapchain_surface_format.format;
	colorAttachment.samples = msaa_sample_count;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = max_msaa_sample_count > 1 ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = vulkan_utils::get_depth_format(context->physical_device);
	depthAttachment.samples = msaa_sample_count;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = swapchain_surface_format.format;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef{};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = msaa_sample_count > 1 ? &colorAttachmentResolveRef : nullptr;
	subpass.inputAttachmentCount = 0;                            // Input attachments can be used to sample from contents of a previous subpass
	subpass.pInputAttachments = nullptr;                         // (Input attachments not used by this example)
	subpass.preserveAttachmentCount = 0;                         // Preserved attachments can be used to loop (and preserve) attachments through subpasses
	subpass.pPreserveAttachments = nullptr;                      // (Preserve attachments not used by this example)

	std::array<VkSubpassDependency, 2> dependencies;
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;                             // Producer of the dependency
	dependencies[0].dstSubpass = 0;                                               // Consumer is our single subpass that will wait for the execution depdendency
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Match our pWaitDstStageMask when we vkQueueSubmit
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // is a loadOp stage for color attachments
	dependencies[0].srcAccessMask = 0;                                            // semaphore wait already does memory dependency for us
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;         // is a loadOp CLEAR access mask for color attachments
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;                                               // Producer of the dependency is our single subpass
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;                             // Consumer are all commands outside of the renderpass
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // is a storeOp stage for color attachments
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;          // Do not block any subsequent work
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;         // is a storeOp `STORE` access mask for color attachments
	dependencies[1].dstAccessMask = 0;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	std::vector<VkAttachmentDescription> attachments;

	if (msaa_sample_count > 1)
	{
		attachments.push_back(colorAttachment);
		attachments.push_back(depthAttachment);
		attachments.push_back(colorAttachmentResolve);
	}
	else
	{
		attachments.push_back(colorAttachment);
		attachments.push_back(depthAttachment);
	}
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();
	VK_ENSURE(vkCreateRenderPass(context->logical_device, &renderPassInfo, vulkan_common::allocation_callback, &render_pass), "Failed to create render pass");
}

void Window::destroy_render_pass()
{
	logger::log("Destroy Render pass");
	vkDestroyRenderPass(context->logical_device, render_pass, vulkan_common::allocation_callback);
	render_pass = VK_NULL_HANDLE;
}

void Window::destroy_window_surface()
{
	logger::log("Destroy window surface");
	vkDestroySurfaceKHR(vulkan_common::instance, surface, vulkan_common::allocation_callback);
}
