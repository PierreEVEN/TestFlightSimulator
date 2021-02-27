#pragma once

#include "rendering/vulkan/utils.h"

class Window;

class ImGuiInstance
{
public:

	ImGuiInstance(Window* context);
	virtual ~ImGuiInstance();


private:

	void init();
	void destroy();


	Window* context;

	VkDescriptorPool descriptor_pool;
	
};

