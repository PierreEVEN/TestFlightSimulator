#pragma once

#include "rendering/gfx_context.h"
#include <memory>

#include "vulkan/command_pool.h"

#include <functional>

class Node;
class SceneNode;
class WindowBase;
class DescriptorPool;
class ImGuiInstance;
class Framebuffer;
class Swapchain;

struct WindowParameters
{
    int         size_x           = 800;
    int         size_y           = 600;
    bool        b_is_fullscreen  = false;
    std::string application_name = "Empty name";
};

struct RenderContext
{
    bool            is_valid       = false;
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkFramebuffer   framebuffer    = VK_NULL_HANDLE;
    uint32_t        image_index    = 0;
};

class Window
{
    friend WindowBase;

  public:
    Window(WindowParameters window_parameters = WindowParameters{});
    virtual ~Window();

    bool begin_frame();
    bool end_frame();

    [[nodiscard]] DescriptorPool*                       get_descriptor_pool() const { return descriptor_pool; }
    [[nodiscard]] VkCommandPool                         get_command_pool() const { return command_pool->get(); }
    [[nodiscard]] GLFWwindow*                           get_handle() const { return window_handle; }
    [[nodiscard]] GfxContext*                           get_gfx_context() const { return gfx_context.get(); }
    [[nodiscard]] VkSurfaceKHR                          get_surface() const { return surface; }
    [[nodiscard]] uint32_t                              get_image_count() const { return swapchain_image_count; }
    [[nodiscard]] vulkan_utils::SwapchainSupportDetails get_support_details() const { return swapchain_support_details; }
    [[nodiscard]] VkSurfaceFormatKHR                    get_surface_format() const { return swapchain_surface_format; }
    [[nodiscard]] VkPresentModeKHR                      get_present_mode() const { return swapchain_present_mode; }
    [[nodiscard]] VkRenderPass                          get_render_pass() const { return render_pass; }

    [[nodiscard]] uint32_t get_msaa_sample_count() const { return msaa_sample_count; }
    [[nodiscard]] uint32_t get_max_msaa_sample_count() const { return max_msaa_sample_count; }

    [[nodiscard]] uint32_t get_height() const { return window_height; }
    [[nodiscard]] uint32_t get_width() const { return window_width; }

    Node* TEMP_NODE = nullptr;

    void resize_window(int res_x, int res_y);
    bool bHasViewportBeenResized = false;

    void          wait_init_idle();
    RenderContext prepare_frame();
    void          prepare_ui(RenderContext& render_context);
    void          render_data(RenderContext& render_context);

  private:
    std::unique_ptr<GfxContext> gfx_context;

    GLFWwindow*                           window_handle;
    command_pool::Container*              command_pool;
    vulkan_utils::SwapchainSupportDetails swapchain_support_details;
    VkSurfaceFormatKHR                    swapchain_surface_format;
    VkPresentModeKHR                      swapchain_present_mode;
    VkSampleCountFlagBits                 max_msaa_sample_count;
    VkSampleCountFlagBits                 msaa_sample_count;
    uint32_t                              swapchain_image_count;

    DescriptorPool* descriptor_pool;
    ImGuiInstance*  imgui_instance = nullptr;

    std::vector<VkSemaphore> image_acquire_semaphore;
    std::vector<VkSemaphore> render_finished_semaphores;
    std::vector<VkFence>     in_flight_fences;
    std::vector<VkFence>     images_in_flight;
    size_t                   current_frame_id = 0;

    uint32_t                     window_width;
    uint32_t                     window_height;
    const char*                  window_name;
    VkSurfaceKHR                 surface     = VK_NULL_HANDLE;
    VkRenderPass                 render_pass = VK_NULL_HANDLE;
    Framebuffer*                 back_buffer;
    std::vector<VkCommandBuffer> command_buffers;

    friend void framebuffer_size_callback(GLFWwindow* handle, int res_x, int res_y);
    void        create_window_surface();
    void        setup_swapchain_property();
    void        create_or_recreate_render_pass();
    void        create_command_buffer();
    void        create_fences_and_semaphores();

    void destroy_fences_and_semaphores();
    void destroy_command_buffer();
    void destroy_render_pass();
    void destroy_window_surface();
};
