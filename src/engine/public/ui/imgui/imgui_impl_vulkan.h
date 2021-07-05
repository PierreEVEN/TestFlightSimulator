// dear imgui: Renderer for Vulkan
// This needs to be used along with a Platform Binding (e.g. GLFW, SDL, Win32, custom..)

// Implemented features:
//  [X] Renderer: Support for large meshes (64k+ vertices) with 16-bit indices.
// In this binding, ImTextureID is used to store a 'VkDescriptorSet' texture identifier. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

// The aim of imgui_impl_vulkan.h/.cpp is to be usable in your engine without any modification.
// IF YOU FEEL YOU NEED TO MAKE ANY CHANGE TO THIS CODE, please share them and your feedback at https://github.com/ocornut/imgui/

// Important note to the reader who wish to integrate imgui_impl_vulkan.cpp/.h in their own engine/app.
// - Common ImGui_ImplVulkan_XXX functions and structures are used to interface with imgui_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering back-end in your engine/app.
// - Helper ImGui_ImplVulkanH_XXX functions and structures are only used by this example (main.cpp) and by
//   the back-end itself (imgui_impl_vulkan.cpp), but should PROBABLY NOT be used by your own engine/app code.
// Read comments in imgui_impl_vulkan.h.

#pragma once

#include "imgui.h"
#include "rendering/vulkan/utils.h"

struct ImDrawData;
class Window;
// Initialization data, for ImGui_ImplVulkan_Init()
// [Please zero-clear before use!]

// Called by user code

// Reusable buffers used for rendering 1 val in-flight frame, for ImGui_ImplVulkan_RenderDrawData()
// [Please zero-clear before use!]
struct ImGui_ImplVulkanH_FrameRenderBuffers
{
    VkDeviceMemory VertexBufferMemory = VK_NULL_HANDLE;
    VkDeviceMemory IndexBufferMemory  = VK_NULL_HANDLE;
    VkDeviceSize   VertexBufferSize   = 0;
    VkDeviceSize   IndexBufferSize    = 0;
    VkBuffer       VertexBuffer       = VK_NULL_HANDLE;
    VkBuffer       IndexBuffer        = VK_NULL_HANDLE;
};

// Each viewport will hold 1 ImGui_ImplVulkanH_WindowRenderBuffers
// [Please zero-clear before use!]
struct ImGui_ImplVulkanH_WindowRenderBuffers
{
    uint32_t                              Index;
    uint32_t                              Count;
    ImGui_ImplVulkanH_FrameRenderBuffers* FrameRenderBuffers = nullptr;
};

class ImGuiInstance
{

  public:
    void ImGui_ImplVulkan_RenderDrawData(ImDrawData* draw_data, VkCommandBuffer command_buffer);
    explicit ImGuiInstance(Window* context);
    ~ImGuiInstance();

  private:
    void        ImGui_ImplVulkanH_DestroyFrameRenderBuffers(VkDevice device, ImGui_ImplVulkanH_FrameRenderBuffers* buffers, const VkAllocationCallbacks* allocator);
    void        ImGui_ImplVulkanH_DestroyWindowRenderBuffers(VkDevice device, ImGui_ImplVulkanH_WindowRenderBuffers* buffers, const VkAllocationCallbacks* allocator);
    bool        ImGui_ImplVulkan_Init(Window* info);
    void        ImGui_ImplVulkan_Shutdown();
    void        CreateOrResizeBuffer(VkBuffer& buffer, VkDeviceMemory& buffer_memory, VkDeviceSize& p_buffer_size, size_t new_size, VkBufferUsageFlagBits usage);
    void        ImGui_ImplVulkan_SetupRenderState(ImDrawData* draw_data, VkCommandBuffer command_buffer, ImGui_ImplVulkanH_FrameRenderBuffers* rb, int fb_width, int fb_height);
    bool        ImGui_ImplVulkan_CreateFontsTexture(VkCommandBuffer command_buffer);
    bool        ImGui_ImplVulkan_CreateDeviceObjects();
    void        ImGui_ImplVulkan_DestroyFontUploadObjects();
    void        ImGui_ImplVulkan_DestroyDeviceObjects();
    void        ImGui_ImplVulkan_SetMinImageCount(uint32_t min_image_count); // To override MinImageCount after initialization (e.g. if swap chain is recreated)
    ImTextureID ImGui_ImplVulkan_AddTexture(VkSampler sampler, VkImageView image_view, VkImageLayout image_layout);

    // Vulkan data
    Window*               g_window_context        = {};
    VkPipelineCache       g_pipeline_cache        = VK_NULL_HANDLE;
    VkDeviceSize          g_BufferMemoryAlignment = 256;
    VkPipelineCreateFlags g_PipelineCreateFlags   = 0x00;
    VkDescriptorSetLayout g_DescriptorSetLayout   = VK_NULL_HANDLE;
    VkPipelineLayout      g_PipelineLayout        = VK_NULL_HANDLE;
    VkPipeline            g_Pipeline              = VK_NULL_HANDLE;

    // Font data
    VkSampler      g_FontSampler        = VK_NULL_HANDLE;
    VkDeviceMemory g_FontMemory         = VK_NULL_HANDLE;
    VkImage        g_FontImage          = VK_NULL_HANDLE;
    VkImageView    g_FontView           = VK_NULL_HANDLE;
    VkDeviceMemory g_UploadBufferMemory = VK_NULL_HANDLE;
    VkBuffer       g_UploadBuffer       = VK_NULL_HANDLE;

    // Render buffers
    ImGui_ImplVulkanH_WindowRenderBuffers g_MainWindowRenderBuffers;
};