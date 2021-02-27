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

#include "rendering/vulkan/utils.h"

class Window;
// Initialization data, for ImGui_ImplVulkan_Init()
// [Please zero-clear before use!]

// Called by user code



bool     ImGui_ImplVulkan_Init(Window* info, VkDescriptorPool desc_pool, VkRenderPass render_pass);
void     ImGui_ImplVulkan_Shutdown();
void     ImGui_ImplVulkan_RenderDrawData(ImDrawData* draw_data, VkCommandBuffer command_buffer);
bool     ImGui_ImplVulkan_CreateFontsTexture(VkCommandBuffer command_buffer);
void     ImGui_ImplVulkan_DestroyFontUploadObjects();
void     ImGui_ImplVulkan_SetMinImageCount(uint32_t min_image_count); // To override MinImageCount after initialization (e.g. if swap chain is recreated)
ImTextureID    ImGui_ImplVulkan_AddTexture(VkSampler sampler, VkImageView image_view, VkImageLayout image_layout);
