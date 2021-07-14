

#include "engine_interface.h"

#include "assets/asset_base.h"
#include "imgui.h"
#include "statsRecorder.h"

static std::shared_ptr<IEngineInterface> engine_interface_reference;

void IEngineInterface::close()
{
    glfwSetWindowShouldClose(get_window()->get_handle(), true);
}

IEngineInterface::IEngineInterface() : last_delta_second_time(std::chrono::steady_clock::now())
{
}

void IEngineInterface::store(const std::shared_ptr<IEngineInterface>& in_engine_interface)
{
    engine_interface_reference = in_engine_interface;
}

IEngineInterface* IEngineInterface::get_internal()
{
    return engine_interface_reference.get();
}

void IEngineInterface::run_main_task(WindowParameters window_parameters)
{
    game_window    = std::make_unique<Window>(window_parameters);
    window_manager = std::make_unique<WindowManager>();
    AssetManager::initialize<AssetManager>();
    input_manager = std::make_unique<InputManager>(game_window->get_handle());

    load_resources();

    pre_initialize();

    while (game_window->begin_frame())
    {
        input_manager->poll_events(get_delta_second());

        const auto now         = std::chrono::steady_clock::now();
        delta_second           = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(now - last_delta_second_time).count()) / 1000000000.0;
        last_delta_second_time = now;

        BEGIN_NAMED_RECORD(DRAW_FRAME);
        game_window->wait_init_idle();
        BEGIN_NAMED_RECORD(PRE_DRAW);
        END_NAMED_RECORD(PRE_DRAW);
        auto render_context = game_window->prepare_frame();
        pre_draw();
        if (render_context.is_valid)
        {
            BEGIN_NAMED_RECORD(RENDER_SCENE);
            render_scene(render_context);
            END_NAMED_RECORD(RENDER_SCENE);
            BEGIN_NAMED_RECORD(DRAW_UI);
            game_window->prepare_ui(render_context);

            ImGui::SetNextWindowPos(ImVec2(-4, -4));
            ImGui::SetNextWindowSize(ImVec2(static_cast<float>(get_window()->get_width() + 8.f), static_cast<float>(get_window()->get_height()) + 8.f));
            if (ImGui::Begin("BackgroundHUD", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground))
            {
                ImGui::DockSpace(ImGui::GetID("Master dockSpace"), ImVec2(0.f, 0.f), ImGuiDockNodeFlags_PassthruCentralNode);
            }
            ImGui::End();
            ImGui::SetNextWindowPos(ImVec2(-4, -4));
            ImGui::SetNextWindowSize(ImVec2(static_cast<float>(get_window()->get_width() + 8.f), static_cast<float>(get_window()->get_height()) + 8.f));
            if (ImGui::Begin("BackgroundHUD", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground))
            {
                render_hud();
            }
            ImGui::End();

            window_manager->draw();
            render_ui();

            END_NAMED_RECORD(DRAW_UI);

            BEGIN_NAMED_RECORD(RENDER_DATA);
            game_window->render_data(render_context);
            END_NAMED_RECORD(RENDER_DATA);
        }
        post_draw();
        game_window->end_frame();
        END_NAMED_RECORD(DRAW_FRAME);
    }
    vkDeviceWaitIdle(GfxContext::get()->logical_device);

    unload_resources();
    AssetManager::destroy();

    pre_shutdown();
    window_manager = nullptr;
    game_window    = nullptr;
}
