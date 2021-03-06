
#include "ui/window/windows/content_browser.h"


#include "engine_interface.h"
#include "assets/asset_base.h"
#include "imgui.h"

void ContentBrowser::draw_content()
{
    if (!get_context() || !get_context()->get_asset_manager())
    {
        ImGui::Text("failed to find content browser !");
        return;
    }
    const auto& content = get_context()->get_asset_manager()->get_assets();
    for (auto& item : content) { ImGui::Text("%s : %s", item.first.to_string().c_str(), item.second->try_load() ? "ready" : "loading"); }
}
