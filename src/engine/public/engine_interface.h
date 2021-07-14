#pragma once
#include "rendering/window.h"

#include "assets/asset_base.h"
#include "ios/input_manager.h"
#include "ui/window/window_base.h"

#include <filesystem>
#include <memory>

class InputManager;
class PlayerController;

class IEngineInterface
{
  public:
    template <typename IEngineInterface_T = IEngineInterface> static IEngineInterface_T* get()
    {
        return static_cast<IEngineInterface_T*>(get_internal());
    }

    template <typename Interface_T> static void run(WindowParameters window_parameters = {})
    {
        auto new_interface = std::make_shared<Interface_T>();
        store(new_interface);
        new_interface->run_main_task(window_parameters);
    }

    [[nodiscard]] virtual std::filesystem::path get_default_font_name() const
    {
        return "None";
    }
    [[nodiscard]] virtual PlayerController* get_controller() = 0;
    
    [[nodiscard]] Window* get_window() const
    {
        return game_window.get();
    }

    [[nodiscard]] InputManager* get_input_manager() const
    {
        return input_manager.get();
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

    virtual void pre_draw()                                 = 0;
    virtual void render_scene(RenderContext render_context) = 0;
    virtual void post_draw()                                = 0;
    virtual void render_ui()                                = 0;
    virtual void render_hud()                               = 0;

    IEngineInterface();

  private:
    static void                           store(const std::shared_ptr<IEngineInterface>& in_engine_interface);
    static IEngineInterface*              get_internal();
    void                                  run_main_task(WindowParameters window_parameters);
    double                                delta_second = 0.0;
    std::chrono::steady_clock::time_point last_delta_second_time;
    std::unique_ptr<Window>               game_window    = nullptr;
    std::unique_ptr<WindowManager>        window_manager = nullptr;
    std::unique_ptr<InputManager>         input_manager  = nullptr;
};
