
#include "ui/window/windowBase.h"


#include "rendering/window.h"
#include "ui/imgui/imgui.h"

WindowManager::~WindowManager()
{
	for (auto* window : windows)
	{
		delete window;
	}
}

void WindowManager::draw()
{
	for (int64_t i = windows.size() - 1; i >= 0; --i)
	{
		if (windows[i]->open)
		{
			windows[i]->draw();
		}
		else
		{
			delete windows[i];
			windows.erase(windows.begin() + i);
		}
	}	
}

void WindowManager::add_window(WindowBase* window)
{
	for (window->window_id = 0; window_ids.find(window->window_id) != window_ids.end(); ++window->window_id);


	windows.push_back(window);
}

void WindowManager::remove_window(WindowBase* window)
{
	windows.erase(std::find(windows.begin(), windows.end(), window));
	window_ids.erase(window_ids.find(window->window_id));

	delete window;	
}

WindowBase::WindowBase(Window* context, const std::string& name, WindowBase* parent)
	: open(true), window_name(name)
{
	logger_log("open window %s", name.c_str());
	if (!context->window_manager)
	{
		logger_error("cannot create window : invalid window manager");
		open = false;
		return;
	}
	context->window_manager->add_window(this);

	if (parent)
	{
		parent_window = parent;
		parent_window->children.push_back(this);
	}
}

WindowBase::~WindowBase()
{
	logger_log("close window %s", window_name.c_str());
}

void WindowBase::close()
{
	if (open) {
		open = false;
		for (auto& child : children)
		{
			child->close();
		}		
	}
}

void WindowBase::draw()
{
	if (!open) return;

	if (ImGui::Begin((window_name + "##" + std::to_string(window_id)).c_str(), &open))
	{
		draw_content();
	}
	ImGui::End();
}

void DemoWindow::draw_content()
{
	ImGui::ShowDemoWindow();
}
