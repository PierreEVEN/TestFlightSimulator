#pragma once
#include "rendering/window.h"

#include "assets/assetBase.h"
#include "ui/window/windowBase.h"
#include "scene/scene.h"

#include <filesystem>
#include <memory>

class PlayerController;

class IEngineInterface
{
  public:
    template <typename T> static void run(WindowParameters window_parameters = {})
    {
        std::unique_ptr<T> new_interface = std::make_unique<T>();
        new_interface->run_main_task(window_parameters);
    }

    [[nodiscard]] virtual std::filesystem::path get_default_font_name() const
    {
        return "None";
    }
    [[nodiscard]] virtual PlayerController* get_controller() = 0;

    [[nodiscard]] AssetManager* get_asset_manager();
    [[nodiscard]] GfxContext*   get_gfx_context() const
    {
        return game_window ? game_window->get_gfx_context() : nullptr;
    }
    [[nodiscard]] Window* get_window() const
    {
        return game_window.get();
    }

    [[nodiscard]] WindowManager* get_window_manager() const
    {
        return window_manager.get();
    }
    [[nodiscard]] double get_delta_second() const
    {
        return delta_second;
    }

    void close();

  protected:
    virtual void load_resources()   = 0;
    virtual void pre_initialize()   = 0;
    virtual void pre_shutdown()     = 0;
    virtual void unload_resources() = 0;

    virtual void pre_draw()     = 0;
    virtual void render_scene() = 0;
    virtual void post_draw()    = 0;
    virtual void render_ui()    = 0;
    virtual void render_hud()   = 0;

    IEngineInterface();

  private:
    void                                  run_main_task(WindowParameters window_parameters);
    double                                delta_second = 0.0;
    std::chrono::steady_clock::time_point last_delta_second_time;
    std::unique_ptr<AssetManager>         asset_manager  = nullptr;
    std::unique_ptr<Window>               game_window    = nullptr;
    std::unique_ptr<WindowManager>        window_manager = nullptr;
};
