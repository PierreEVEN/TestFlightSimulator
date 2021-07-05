#pragma once
#include "engine_interface.h"

class TestGameInterface : public IEngineInterface
{
public:
    PlayerController* get_controller() override;

protected:
    void load_resources() override;
    void pre_initialize() override;
    void pre_shutdown() override;
    void unload_resources() override;
    void render_scene(RenderContext render_context) override;
    void render_ui() override;
    void render_hud() override;
    void pre_draw() override;
    void post_draw() override;

    std::unique_ptr<Scene> root_scene;
};

