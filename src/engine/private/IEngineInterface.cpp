

#include "IEngineInterface.h"

#include "assets/assetBase.h"
#include "imgui.h"

AssetManager* IEngineInterface::get_asset_manager() { return asset_manager.get(); }

void IEngineInterface::run_main_task(WindowParameters window_parameters)
{
    game_window    = std::make_unique<Window>(window_parameters);
    window_manager = std::make_unique<WindowManager>();
    asset_manager  = std::make_unique<AssetManager>(this);

    load_resources();

    pre_initialize();

    while (game_window->begin_frame())
    {
        game_window->wait_init_idle();
        pre_draw();
        auto render_context = game_window->prepare_frame();
        if (render_context.is_valid)
        {
            render_scene();
            game_window->prepare_ui(render_context);

            ImGui::SetNextWindowPos(ImVec2(-4, -4));
            ImGui::SetNextWindowSize(ImVec2(static_cast<float>(get_window()->get_width() + 8.f), static_cast<float>(get_window()->get_height()) + 8.f));
            if (ImGui::Begin("BackgroundHUD", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground))
            {
                ImGui::DockSpace(ImGui::GetID("Master dockSpace"), ImVec2(0.f, 0.f), ImGuiDockNodeFlags_PassthruCentralNode);
                render_hud();
            }
            ImGui::End();
            render_ui();
            window_manager->draw();

            game_window->render_data(render_context);
        }
        post_draw();
        game_window->end_frame();
    }
    unload_resources();
    asset_manager = nullptr;

    pre_shutdown();
    window_manager = nullptr;
    game_window    = nullptr;
}
