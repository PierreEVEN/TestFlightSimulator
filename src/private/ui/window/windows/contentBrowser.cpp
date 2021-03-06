
#include "ui/window/windows/contentBrowser.h"


#include "assets/assetBase.h"
#include "rendering/window.h"
#include "ui/imgui/imgui.h"

void ContentBrowser::draw_content()
{
	if (!get_context() || !get_context()->get_asset_manager())
	{
		ImGui::Text("failed to find content browser !");
		return;
	}
	auto content = get_context()->get_asset_manager()->get_assets();
	for (auto& item : content)
	{
		ImGui::Text("%s : %s", item.first.to_string().c_str(), item.second->try_load() ? "ready" : "loading");
	}	
}
