#include "ui/imgui/imgui_instance.h"

#include <array>



#include "rendering/window.h"
#include "ui/imgui/imgui.h"
#include "ui/imgui/imgui_impl_glfw.h"
#include "ui/imgui/imgui_impl_vulkan.h"

void ImGuiVkResultDelegate(VkResult err)
{
	if (err != VK_SUCCESS) { logger_fail("ImGui initialization error : %s", err); }
}

void ImGuiInstance::init()
{
	logger_log("Initialize imgui ressources");

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0;
	style.ScrollbarRounding = 0;
	style.TabRounding = 0;
	style.WindowBorderSize = 1;
	style.PopupBorderSize = 1;
	style.WindowTitleAlign.x = 0.5f;
	style.FramePadding.x = 6.f;
	style.FramePadding.y = 6.f;
	style.WindowPadding.x = 4.f;
	style.WindowPadding.y = 4.f;
	style.GrabMinSize = 16.f;
	style.ScrollbarSize = 20.f;
	style.IndentSpacing = 30.f;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	//@TODO set font
	//G_IMGUI_DEFAULT_FONT = io.Fonts->AddFontFromFileTTF(G_DEFAULT_FONT_PATH.GetValue().GetData(), 20.f);


	const size_t descPoolCount = 20;

	std::array<VkDescriptorPoolSize, 11> pool_sizes;
	pool_sizes[0] = { VK_DESCRIPTOR_TYPE_SAMPLER, descPoolCount };
	pool_sizes[1] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descPoolCount };
	pool_sizes[2] = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, descPoolCount };
	pool_sizes[3] = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descPoolCount };
	pool_sizes[4] = { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, descPoolCount };
	pool_sizes[5] = { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, descPoolCount };
	pool_sizes[6] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descPoolCount };
	pool_sizes[7] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descPoolCount };
	pool_sizes[8] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, descPoolCount };
	pool_sizes[9] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, descPoolCount };
	pool_sizes[10] = { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, descPoolCount };

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	poolInfo.pPoolSizes = pool_sizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(context->get_image_count());

	VK_ENSURE(vkCreateDescriptorPool(context->get_context()->logical_device, &poolInfo, vulkan_common::allocation_callback, &descriptor_pool), "Failed to create imgui descriptor pool");

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForVulkan(context->get_handle(), true);

	ImGui_ImplVulkan_Init(context, descriptor_pool, context->get_render_pass());

	// Upload Fonts
	{
		VkCommandBuffer command_buffer = vulkan_utils::begin_single_time_commands(context);
		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
		vulkan_utils::end_single_time_commands(context, command_buffer);
	}
}

void ImGuiInstance::destroy()
{
	logger_log("cleaning up ImGui ressources");
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();

	vkDestroyDescriptorPool(context->get_context()->logical_device, descriptor_pool, vulkan_common::allocation_callback);
}

ImGuiInstance::ImGuiInstance(Window* window_context)
	: context(window_context)
{
	init();
}

ImGuiInstance::~ImGuiInstance()
{
	destroy();
}
