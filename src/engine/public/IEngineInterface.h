#pragma once
#include "rendering/window.h"

#include "assets/assetBase.h"
#include "ui/window/windowBase.h"

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

    virtual std::filesystem::path get_default_font_name() const { return "None"; }
    virtual PlayerController*     get_controller() = 0;

    AssetManager*  get_asset_manager();
    GfxContext*    get_gfx_context() const { return game_window ? game_window->get_gfx_context() : nullptr; }
    Window*        get_window() const { return game_window.get(); }
    WindowManager* get_window_manager() const { return window_manager.get(); }

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

  private:
    void                           run_main_task(WindowParameters window_parameters);
    std::unique_ptr<AssetManager>  asset_manager  = nullptr;
    std::unique_ptr<Window>        game_window    = nullptr;
    std::unique_ptr<WindowManager> window_manager = nullptr;
};
