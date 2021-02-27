
#include "rendering/window.h"


#include <mutex>
#include <set>
#include <unordered_map>
#include <array>

#include "config.h"
#include "ios/logger.h"
#include "rendering/vulkan/commandPool.h"
#include "rendering/vulkan/framebuffer.h"
#include "rendering/vulkan/swapchain.h"
#include "ui/imgui/imgui_impl_glfw.h"
#include "ui/imgui/imgui_impl_vulkan.h"

std::mutex test;
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
	logger_log("destroy window window");

	destroy_vma_allocators();
	destroy_logical_device();
}

Window::Window(const int res_x, const int res_y, const char* name, bool fullscreen, bool img_context)
	: window_width(res_x), window_height(res_y), window_name(name), has_imgui_context(img_context)
{
	std::lock_guard<std::mutex> lock(window_map_lock);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	if (fullscreen) glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	// Create window handle
	window_handle = glfwCreateWindow(res_x, res_y, name, nullptr, nullptr);
	if (!window_handle) logger_fail("failed to create glfw Window");
	glfwSetFramebufferSizeCallback(window_handle, framebuffer_size_callback);

	// Create window surface
	create_window_surface();

	// Get vulkan window from existing window or create a new one
	if (!window_map.empty()) {
		logger_log("use window from '%s' window", window_map.begin()->second->window_name);
		context = window_map.begin()->second->context;
		vulkan_utils::find_device_queue_families(surface, context->physical_device);
	}
	else {
		logger_log("create new vulkan window");
		context = std::make_shared<WindowContext>(window_handle, surface);
	}

	// Create window vulkan objects
	command_pool = new command_pool::Container(context->logical_device, context->queue_families.graphic_family.value());
	setup_swapchain_property();
	create_or_recreate_render_pass();

	back_buffer = new Framebuffer(this, VkExtent2D{ static_cast<uint32_t>(window_width), static_cast<uint32_t>(window_height) });
	create_command_buffer();
	create_fences_and_semaphores();

	if (has_imgui_context) imgui_instance = new ImGuiInstance(this);
	
	window_map[window_handle] = this;
	
	logger_validate("created Window '%s' ( %d x %d )", name, res_x, res_y);
}

Window::~Window() {
	std::lock_guard<std::mutex> lock(window_map_lock);
	window_map.erase(window_map.find(window_handle));

	context->wait_device();
	
	if (has_imgui_context) delete imgui_instance;
	
	destroy_fences_and_semaphores();
	destroy_command_buffer();
	delete back_buffer;
	destroy_render_pass();
	delete command_pool;
	context = nullptr;
	destroy_window_surface();
	
	glfwDestroyWindow(window_handle);
	logger_validate("successfully destroyed window");
}

void Window::setup_swapchain_property()
{
	swapchain_support_details = vulkan_utils::get_swapchain_support_details(surface, context->physical_device);
	swapchain_surface_format = vulkan_utils::choose_swapchain_surface_format(context->physical_device, surface);
	swapchain_present_mode = vulkan_utils::choose_swapchain_present_mode(swapchain_support_details.present_modes);
	swapchain_image_count = swapchain_support_details.capabilities.minImageCount + 1;
	max_msaa_sample_count = vulkan_utils::get_max_usable_sample_count(context->physical_device);
	msaa_sample_count = max_msaa_sample_count;
	logger_log("swapchain details : \n\
		\t-max samples : %d\n\
		\t-image count : %d\n\
		\t-present mode : %d\n\
		\t-surface log_format : %d\
		", max_msaa_sample_count, swapchain_image_count, swapchain_present_mode, swapchain_surface_format);
}

void Window::resize_window(const int res_x, const int res_y) {
	logger_log("resize window");
	
	window_width = res_x;
	window_height = res_y;

	context->wait_device();

	back_buffer->set_size(VkExtent2D{ static_cast<uint32_t>(res_x),static_cast<uint32_t>(res_y) });
}

bool Window::begin_frame()
{
	glfwPollEvents();
	return !glfwWindowShouldClose(window_handle);
}

bool Window::end_frame()
{
	render();
	return !glfwWindowShouldClose(window_handle);
}

void Window::create_window_surface()
{
	VK_ENSURE(glfwCreateWindowSurface(vulkan_common::instance, window_handle, nullptr, &surface) != VK_SUCCESS, "Failed to create Window surface");
	VK_CHECK(surface, "VkSurfaceKHR is null");
	logger_log("Create Window surface");
}

void WindowContext::submit_graphic_queue(const VkSubmitInfo& submit_infos, VkFence& submit_fence)
{
	std::lock_guard<std::mutex> lock(queue_access_lock);
	VK_ENSURE(vkQueueSubmit(graphic_queue, 1, &submit_infos, submit_fence), "Failed to submit graphic queue");
}

VkResult WindowContext::submit_present_queue(const VkPresentInfoKHR& present_infos)
{
	std::lock_guard<std::mutex> lock(queue_access_lock);
	return vkQueuePresentKHR(present_queue, &present_infos);
}

void WindowContext::wait_device()
{
	std::lock_guard<std::mutex> queue_lock(queue_access_lock);
	vkDeviceWaitIdle(logical_device);
}

void WindowContext::select_physical_device(VkSurfaceKHR surface)
{
	// Get devices
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(vulkan_common::instance, &device_count, nullptr);
	if (device_count == 0) logger_fail("No graphical device found.");
	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(vulkan_common::instance, &device_count, devices.data());

	// Enumerate devices
	std::string PhysLog = "Found " + std::to_string(device_count) + " graphical devices : \n";
	for (const VkPhysicalDevice& device : devices) {
		VkPhysicalDeviceProperties pProperties;
		vkGetPhysicalDeviceProperties(device, &pProperties);
		PhysLog += "\t-" + std::string(pProperties.deviceName) + " (driver version : " + std::to_string(pProperties.driverVersion) + ")\n";
	}
	logger_log(PhysLog.c_str());

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
	logger_log("Picking physical device %d (%s)", selected_device_properties.deviceID, selected_device_properties.deviceName);
}

void WindowContext::create_logical_device(VkSurfaceKHR surface)
{
	logger_log("Create logical device");

	queue_families = vulkan_utils::find_device_queue_families(surface, physical_device);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> unique_queue_families = { queue_families.graphic_family.value(), queue_families.present_family.value() };
	float queue_priorities = 1.0f;
	for (uint32_t queueFamily : unique_queue_families) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queue_priorities;
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
	logger_log("Create memory allocators");
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = physical_device;
	allocatorInfo.device = logical_device;
	allocatorInfo.instance = vulkan_common::instance;

	vmaCreateAllocator(&allocatorInfo, &vulkan_memory_allocator);
}

void WindowContext::destroy_vma_allocators()
{
	logger_log("Destroy memory allocators");
	vmaDestroyAllocator(vulkan_memory_allocator);
}

void WindowContext::destroy_logical_device()
{
	logger_log("Destroy logical device");
	vkDestroyDevice(logical_device, vulkan_common::allocation_callback);
}

void Window::create_or_recreate_render_pass()
{
	if (render_pass != VK_NULL_HANDLE) {
		destroy_render_pass();
	}

	logger_log("Create render pass");
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

void Window::create_command_buffer()
{
	logger_log("create command buffers");
	command_buffers.resize(swapchain_image_count);
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = command_pool->get();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

	VK_ENSURE(vkAllocateCommandBuffers(context->logical_device, &allocInfo, command_buffers.data()), "Failed to allocate command buffer");
}

void Window::create_fences_and_semaphores()
{
	logger_log("create fence and semaphores\n\t-in flight fence : %d\n\t-images in flight : %d", config::max_frame_in_flight, swapchain_image_count);
	image_acquire_semaphore.resize(config::max_frame_in_flight);
	render_finished_semaphores.resize(config::max_frame_in_flight);
	in_flight_fences.resize(config::max_frame_in_flight);
	images_in_flight.resize(swapchain_image_count, VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < config::max_frame_in_flight; i++) {
		VK_ENSURE(vkCreateSemaphore(context->logical_device, &semaphoreInfo, vulkan_common::allocation_callback, &image_acquire_semaphore[i]), "Failed to create image available semaphore #%d" + i);
		VK_ENSURE(vkCreateSemaphore(context->logical_device, &semaphoreInfo, vulkan_common::allocation_callback, &render_finished_semaphores[i]), "Failed to create render finnished semaphore #%d" + i)
		VK_ENSURE(vkCreateFence(context->logical_device, &fenceInfo, vulkan_common::allocation_callback, &in_flight_fences[i]), "Failed to create fence #%d" + i);
	}
}
void Window::render()
{
	/**
	 * Select available handles for next image
	 */
	
	// Select next image
	current_frame_id = (current_frame_id + 1) % config::max_frame_in_flight;
	
	// Ensure all frame data are submitted
	vkWaitForFences(context->logical_device, 1, &in_flight_fences[current_frame_id], VK_TRUE, UINT64_MAX);

	// Retrieve the next available image ID
	uint32_t image_index;
	VkResult result = vkAcquireNextImageKHR(context->logical_device, back_buffer->get_swapchain()->get(), UINT64_MAX, image_acquire_semaphore[current_frame_id], VK_NULL_HANDLE, &image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		int width = 0, height = 0;
		glfwGetFramebufferSize(window_handle, &width, &height);
		resize_window(width, height);
		return;
	}
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		logger_error("Failed to acquire image from the swapchain");
		return;
	}

	if (images_in_flight[image_index] != VK_NULL_HANDLE) vkWaitForFences(context->logical_device, 1, &images_in_flight[image_index], VK_TRUE, UINT64_MAX);
	images_in_flight[image_index] = in_flight_fences[current_frame_id];
	
	const VkCommandBuffer current_command_buffer = command_buffers[image_index];
	const VkFramebuffer current_framebuffer = back_buffer->get(image_index);
	

	/**
	 * Build command queues
	 */

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = 0; // Optional
	begin_info.pInheritanceInfo = nullptr; // Optional
	
	if (vkBeginCommandBuffer(current_command_buffer, &begin_info) != VK_SUCCESS) { logger_fail("Failed to create command buffer #%d", image_index); }

	std::array<VkClearValue, 2> clear_values{};
	clear_values[0].color = { 0.6f, 0.9f, 1.f, 1.0f };
	clear_values[1].depthStencil = { 1.0f, 0 };
	
	VkRenderPassBeginInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = render_pass;
	render_pass_info.framebuffer = current_framebuffer;
	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = VkExtent2D{ window_width, window_height };
	render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
	render_pass_info.pClearValues = clear_values.data();
	
	vkCmdBeginRenderPass(current_command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	// Set viewport and scissor params
	VkViewport viewport;
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = static_cast<float>(window_width);
	viewport.height = static_cast<float>(window_height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor;
	scissor.extent = VkExtent2D{ window_width, window_height };
	scissor.offset = VkOffset2D{ 0, 0 };	
	vkCmdSetViewport(current_command_buffer, 0, 1, &viewport);
	vkCmdSetScissor(current_command_buffer, 0, 1, &scissor);




	





	/************************************************************************/
	/* Begin imgui draw stuff                                               */
	/************************************************************************/
	
	if (has_imgui_context) {
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGui::PushFont(G_IMGUI_DEFAULT_FONT);

		if (ImGui::BeginMainMenuBar())
		{
			ImGui::EndMainMenuBar();
		}

		ImGui::ShowDemoWindow();
		
		//ImGui::PopFont();
		ImGui::EndFrame();

		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
		imgui_instance->ImGui_ImplVulkan_RenderDrawData(draw_data, current_command_buffer);
	}
	
	/************************************************************************/
	/* End imgui draw stuff                                                 */
	/************************************************************************/




	









	
	vkCmdEndRenderPass(current_command_buffer);
	VK_ENSURE(vkEndCommandBuffer(current_command_buffer), "Failed to register command buffer #d", image_index);


	/**
	 * Submit queues
	 */
	
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore acquire_wait_semaphore[] = { image_acquire_semaphore[current_frame_id] };
	VkPipelineStageFlags wait_stage[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = acquire_wait_semaphore;
	submitInfo.pWaitDstStageMask = wait_stage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &current_command_buffer;

	VkSemaphore finished_semaphore[] = { render_finished_semaphores[current_frame_id] }; // This fence is used to tell when the gpu can present the submitted data
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = finished_semaphore;

	/** Submit command buffers */
	vkResetFences(context->logical_device, 1, &in_flight_fences[current_frame_id]);
	context->submit_graphic_queue(submitInfo, in_flight_fences[current_frame_id]); // Pass fence to know when all the data are submitted


	/**
	 * Present to swapchain
	 */
	
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = finished_semaphore;

	VkSwapchainKHR swapChains[] = { back_buffer->get_swapchain()->get() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &image_index;
	presentInfo.pResults = nullptr; // Optional

	result = context->submit_present_queue(presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || bHasViewportBeenResized) {
		int width = 0, height = 0;
		glfwGetFramebufferSize(window_handle, &width, &height);
		resize_window(width, height);
	}
	else if (result != VK_SUCCESS) {
		logger_fail("Failed to present image to swap chain");
	}
}

void Window::destroy_fences_and_semaphores()
{
	logger_log("destroy fence and semaphores");
	for (size_t i = 0; i < config::max_frame_in_flight; i++) {
		vkDestroySemaphore(context->logical_device, render_finished_semaphores[i], vulkan_common::allocation_callback);
		vkDestroySemaphore(context->logical_device, image_acquire_semaphore[i], vulkan_common::allocation_callback);
		vkDestroyFence(context->logical_device, in_flight_fences[i], vulkan_common::allocation_callback);
	}
}

void Window::destroy_command_buffer()
{
	logger_log("free command buffers");
	vkFreeCommandBuffers(context->logical_device, command_pool->get(), static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
}

void Window::destroy_render_pass()
{
	logger_log("Destroy Render pass");
	vkDestroyRenderPass(context->logical_device, render_pass, vulkan_common::allocation_callback);
	render_pass = VK_NULL_HANDLE;
}

void Window::destroy_window_surface()
{
	logger_log("Destroy window surface");
	vkDestroySurfaceKHR(vulkan_common::instance, surface, vulkan_common::allocation_callback);
}
