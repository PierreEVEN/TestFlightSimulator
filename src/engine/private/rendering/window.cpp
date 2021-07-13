
#include "rendering/window.h"

#include <array>
#include <mutex>
#include <set>
#include <unordered_map>

#include "assets/asset_base.h"
#include "backends/imgui_impl_glfw.h"
#include "config.h"
#include "rendering/vulkan/command_pool.h"
#include "rendering/vulkan/descriptor_pool.h"
#include "rendering/vulkan/framebuffer.h"
#include "rendering/vulkan/swapchain.h"
#include "ui/imgui/imgui_impl_vulkan.h"
#include "ui/window/window_base.h"
#include "ui/window/windows/profiler.h"
#include <cpputils/logger.hpp>

std::mutex                               window_map_lock;
std::unordered_map<GLFWwindow*, Window*> window_map;

void framebuffer_size_callback(GLFWwindow* handle, const int res_x, const int res_y)
{
    auto window_ptr = window_map.find(handle);
    if (window_ptr != window_map.end())
    {
        window_ptr->second->resize_window(res_x, res_y);
    }
}

Window::Window(WindowParameters window_parameters) : window_width(window_parameters.size_x), window_height(window_parameters.size_y), window_name(window_parameters.application_name.c_str())
{
    std::lock_guard<std::mutex> lock(window_map_lock);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    if (window_parameters.b_is_fullscreen)
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    // Create window handle
    window_handle = glfwCreateWindow(window_parameters.size_x, window_parameters.size_y, window_parameters.application_name.c_str(), nullptr, nullptr);
    if (!window_handle)
        LOG_FATAL("failed to create glfw Window");
    glfwSetFramebufferSizeCallback(window_handle, framebuffer_size_callback);

    // Create window surface
    create_window_surface();

    LOG_INFO("create new vulkan window");
    gfx_context = std::make_unique<GfxContext>(surface);

    // Create window vulkan objects
    command_pool = new command_pool::Container(gfx_context->logical_device, gfx_context->queue_families.graphic_family.value());
    LOG_INFO("finished window creation");
    setup_swapchain_property();

    create_or_recreate_render_pass();

    back_buffer = new Framebuffer(this, VkExtent2D{static_cast<uint32_t>(window_width), static_cast<uint32_t>(window_height)});
    create_command_buffer();
    create_fences_and_semaphores();
    descriptor_pool = new DescriptorPool(this);

    imgui_instance = new ImGuiInstance(this);

    window_map[window_handle] = this;

    LOG_VALIDATE("created Window '%s' ( %d x %d )", window_parameters.application_name.c_str(), window_parameters.size_x, window_parameters.size_y);
}

Window::~Window()
{
    std::lock_guard<std::mutex> lock(window_map_lock);
    window_map.erase(window_map.find(window_handle));

    gfx_context->wait_device();

    delete imgui_instance;

    delete descriptor_pool;
    destroy_fences_and_semaphores();
    destroy_command_buffer();
    delete back_buffer;
    destroy_render_pass();
    delete command_pool;
    gfx_context = nullptr;
    destroy_window_surface();

    glfwDestroyWindow(window_handle);
    LOG_VALIDATE("successfully destroyed window");
}

void Window::setup_swapchain_property()
{
    swapchain_support_details = vulkan_utils::get_swapchain_support_details(surface, gfx_context->physical_device);
    swapchain_surface_format  = vulkan_utils::choose_swapchain_surface_format(gfx_context->physical_device, surface);
    swapchain_present_mode    = vulkan_utils::choose_swapchain_present_mode(swapchain_support_details.present_modes);
    swapchain_image_count     = swapchain_support_details.capabilities.minImageCount + 1;
    max_msaa_sample_count     = vulkan_utils::get_max_usable_sample_count(gfx_context->physical_device);
    msaa_sample_count         = max_msaa_sample_count;
    LOG_INFO("swapchain details : \n\
		\t-max pass_samples : %d\n\
		\t-image count : %d\n\
		\t-present mode : %d\n\
		\t-surface log_format : %d\
		",
             max_msaa_sample_count, swapchain_image_count, swapchain_present_mode, swapchain_surface_format);
}

void Window::resize_window(const int res_x, const int res_y)
{
    BEGIN_NAMED_RECORD(RESIZE_WINDOW);
    LOG_INFO("resize window");

    window_width  = res_x;
    window_height = res_y;

    BEGIN_NAMED_RECORD(WAIT_DEVICE);
    gfx_context->wait_device();
    END_NAMED_RECORD(WAIT_DEVICE);

    back_buffer->set_size(VkExtent2D{static_cast<uint32_t>(res_x), static_cast<uint32_t>(res_y)});
}

void Window::wait_init_idle()
{
    BEGIN_NAMED_RECORD(WAIT_INIT_IDLE);
    /**
     * Select available handles for next image
     */

    // Select next image
    current_frame_id = (current_frame_id + 1) % config::max_frame_in_flight;

    // Ensure all frame data are submitted
    vkWaitForFences(gfx_context->logical_device, 1, &in_flight_fences[current_frame_id], VK_TRUE, UINT64_MAX);

    END_NAMED_RECORD(WAIT_INIT_IDLE);
}

RenderContext Window::prepare_frame()
{
    BEGIN_NAMED_RECORD(PREPARE_FRAME);
    // Retrieve the next available image ID
    uint32_t image_index;
    BEGIN_NAMED_RECORD(ACQUIRE_NEXT_IMAGE);
    VkResult result = vkAcquireNextImageKHR(gfx_context->logical_device, back_buffer->get_swapchain()->get(), UINT64_MAX, image_acquire_semaphore[current_frame_id], VK_NULL_HANDLE, &image_index);
    END_NAMED_RECORD(ACQUIRE_NEXT_IMAGE);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window_handle, &width, &height);
        resize_window(width, height);
        return RenderContext{};
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        LOG_ERROR("Failed to acquire image from the swapchain");
        return RenderContext{};
    }

    BEGIN_NAMED_RECORD(WAIT_FOR_FENCES);
    if (images_in_flight[image_index] != VK_NULL_HANDLE)
        vkWaitForFences(gfx_context->logical_device, 1, &images_in_flight[image_index], VK_TRUE, UINT64_MAX);
    images_in_flight[image_index] = in_flight_fences[current_frame_id];
    END_NAMED_RECORD(WAIT_FOR_FENCES);

    RenderContext render_context{
        .is_valid       = true,
        .command_buffer = command_buffers[image_index],
        .framebuffer    = back_buffer->get(image_index),
        .image_index    = image_index,
        .res_x          = get_width(),
        .res_y          = get_height(),
    };

    /**
     * Build command queues
     */

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags            = 0;       // Optional
    begin_info.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(render_context.command_buffer, &begin_info) != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create command buffer #%d", image_index);
    }

    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color        = {0.6f, 0.9f, 1.f, 1.0f};
    clear_values[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass        = render_pass;
    render_pass_info.framebuffer       = render_context.framebuffer;
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = VkExtent2D{window_width, window_height};
    render_pass_info.clearValueCount   = static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues      = clear_values.data();

    vkCmdBeginRenderPass(render_context.command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    // Set viewport and scissor params
    VkViewport viewport;
    viewport.x        = 0;
    viewport.y        = 0;
    viewport.width    = static_cast<float>(window_width);
    viewport.height   = static_cast<float>(window_height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor;
    scissor.extent = VkExtent2D{window_width, window_height};
    scissor.offset = VkOffset2D{0, 0};
    vkCmdSetViewport(render_context.command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(render_context.command_buffer, 0, 1, &scissor);

    END_NAMED_RECORD(PREPARE_FRAME);
    return render_context;
}

void Window::prepare_ui(RenderContext& render_context)
{
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Window::render_data(RenderContext& render_context)
{

    ImGui::EndFrame();

    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

    BEGIN_NAMED_RECORD(RENDER_IMGUI_DATA);
    imgui_instance->ImGui_ImplVulkan_RenderDrawData(draw_data, render_context.command_buffer);
    END_NAMED_RECORD(RENDER_IMGUI_DATA);

    /************************************************************************/
    /* End imgui draw stuff                                                 */
    /************************************************************************/

    vkCmdEndRenderPass(render_context.command_buffer);
    VK_ENSURE(vkEndCommandBuffer(render_context.command_buffer), "Failed to register command buffer #d", render_context.image_index);

    /**
     * Submit queues
     */

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore          acquire_wait_semaphore[] = {image_acquire_semaphore[current_frame_id]};
    VkPipelineStageFlags wait_stage[]             = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount                 = 1;
    submitInfo.pWaitSemaphores                    = acquire_wait_semaphore;
    submitInfo.pWaitDstStageMask                  = wait_stage;
    submitInfo.commandBufferCount                 = 1;
    submitInfo.pCommandBuffers                    = &render_context.command_buffer;

    VkSemaphore finished_semaphore[] = {render_finished_semaphores[current_frame_id]}; // This fence is used to tell when the gpu can present the submitted data
    submitInfo.signalSemaphoreCount  = 1;
    submitInfo.pSignalSemaphores     = finished_semaphore;

    BEGIN_NAMED_RECORD(SUBMIT_QUEUE);
    /** Submit command buffers */
    vkResetFences(gfx_context->logical_device, 1, &in_flight_fences[current_frame_id]);
    gfx_context->submit_graphic_queue(submitInfo, in_flight_fences[current_frame_id]); // Pass fence to know when all the data are submitted

    /**
     * Present to swapchain
     */

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = finished_semaphore;

    VkSwapchainKHR swapChains[] = {back_buffer->get_swapchain()->get()};
    presentInfo.swapchainCount  = 1;
    presentInfo.pSwapchains     = swapChains;
    presentInfo.pImageIndices   = &render_context.image_index;
    presentInfo.pResults        = nullptr; // Optional

    VkResult result = gfx_context->submit_present_queue(presentInfo);
    END_NAMED_RECORD(SUBMIT_QUEUE);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || bHasViewportBeenResized)
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window_handle, &width, &height);
        resize_window(width, height);
    }
    else if (result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to present image to swap chain");
    }
}

bool Window::begin_frame()
{
    return !glfwWindowShouldClose(window_handle);
}

bool Window::end_frame()
{
    return !glfwWindowShouldClose(window_handle);
}

void Window::create_window_surface()
{
    VK_ENSURE(glfwCreateWindowSurface(vulkan_common::instance, window_handle, nullptr, &surface) != VK_SUCCESS, "Failed to create Window surface");
    VK_CHECK(surface, "VkSurfaceKHR is null");
    LOG_INFO("Create Window surface");
}
void Window::create_or_recreate_render_pass()
{
    if (render_pass != VK_NULL_HANDLE)
    {
        destroy_render_pass();
    }

    LOG_INFO("Create render pass");
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format         = swapchain_surface_format.format;
    colorAttachment.samples        = msaa_sample_count;
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = max_msaa_sample_count > 1 ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format         = vulkan_utils::get_depth_format(gfx_context->physical_device);
    depthAttachment.samples        = msaa_sample_count;
    depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format         = swapchain_surface_format.format;
    colorAttachmentResolve.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments     = msaa_sample_count > 1 ? &colorAttachmentResolveRef : nullptr;
    subpass.inputAttachmentCount    = 0;       // Input attachments can be used to sample from contents of a previous subpass
    subpass.pInputAttachments       = nullptr; // (Input attachments not used by this example)
    subpass.preserveAttachmentCount = 0;       // Preserved attachments can be used to loop (and preserve) attachments through subpasses
    subpass.pPreserveAttachments    = nullptr; // (Preserve attachments not used by this example)

    std::array<VkSubpassDependency, 2> dependencies;
    dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;                           // Producer of the dependency
    dependencies[0].dstSubpass      = 0;                                             // Consumer is our single subpass that will wait for the execution depdendency
    dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Match our pWaitDstStageMask when we vkQueueSubmit
    dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // is a loadOp stage for color attachments
    dependencies[0].srcAccessMask   = 0;                                             // semaphore wait already does memory dependency for us
    dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;          // is a loadOp CLEAR access mask for color attachments
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass      = 0;                                             // Producer of the dependency is our single subpass
    dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;                           // Consumer are all commands outside of the renderpass
    dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // is a storeOp stage for color attachments
    dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;          // Do not block any subsequent work
    dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;          // is a storeOp `STORE` access mask for color attachments
    dependencies[1].dstAccessMask   = 0;
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
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments    = attachments.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies   = dependencies.data();
    VK_ENSURE(vkCreateRenderPass(gfx_context->logical_device, &renderPassInfo, vulkan_common::allocation_callback, &render_pass), "Failed to create render pass");
}

void Window::create_command_buffer()
{
    LOG_INFO("create command buffers");
    command_buffers.resize(swapchain_image_count);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = command_pool->get();
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

    VK_ENSURE(vkAllocateCommandBuffers(gfx_context->logical_device, &allocInfo, command_buffers.data()), "Failed to allocate command buffer");
}

void Window::create_fences_and_semaphores()
{
    LOG_INFO("create fence and semaphores\n\t-in flight fence : %d\n\t-images in flight : %d", config::max_frame_in_flight, swapchain_image_count);
    image_acquire_semaphore.resize(config::max_frame_in_flight);
    render_finished_semaphores.resize(config::max_frame_in_flight);
    in_flight_fences.resize(config::max_frame_in_flight);
    images_in_flight.resize(swapchain_image_count, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < config::max_frame_in_flight; i++)
    {
        VK_ENSURE(vkCreateSemaphore(gfx_context->logical_device, &semaphoreInfo, vulkan_common::allocation_callback, &image_acquire_semaphore[i]), "Failed to create image available semaphore #%d" + i);
        VK_ENSURE(vkCreateSemaphore(gfx_context->logical_device, &semaphoreInfo, vulkan_common::allocation_callback, &render_finished_semaphores[i]), "Failed to create render finnished semaphore #%d" + i)
        VK_ENSURE(vkCreateFence(gfx_context->logical_device, &fenceInfo, vulkan_common::allocation_callback, &in_flight_fences[i]), "Failed to create fence #%d" + i);
    }
}

void Window::destroy_fences_and_semaphores()
{
    LOG_INFO("destroy fence and semaphores");
    for (size_t i = 0; i < config::max_frame_in_flight; i++)
    {
        vkDestroySemaphore(gfx_context->logical_device, render_finished_semaphores[i], vulkan_common::allocation_callback);
        vkDestroySemaphore(gfx_context->logical_device, image_acquire_semaphore[i], vulkan_common::allocation_callback);
        vkDestroyFence(gfx_context->logical_device, in_flight_fences[i], vulkan_common::allocation_callback);
    }
}

void Window::destroy_command_buffer()
{
    LOG_INFO("free command buffers");
    vkFreeCommandBuffers(gfx_context->logical_device, command_pool->get(), static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
}

void Window::destroy_render_pass()
{
    LOG_INFO("Destroy Render pass");
    vkDestroyRenderPass(gfx_context->logical_device, render_pass, vulkan_common::allocation_callback);
    render_pass = VK_NULL_HANDLE;
}

void Window::destroy_window_surface()
{
    LOG_INFO("Destroy window surface");
    vkDestroySurfaceKHR(vulkan_common::instance, surface, vulkan_common::allocation_callback);
}
