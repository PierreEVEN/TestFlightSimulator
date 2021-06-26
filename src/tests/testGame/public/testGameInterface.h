#pragma once
#include "IEngineInterface.h"

class TestGameInterface : public IEngineInterface
{
public:
    PlayerController* get_controller() override;

protected:
    void load_resources() override;
    void pre_initialize() override;
    void pre_shutdown() override;
    void unload_resources() override;
    void render_scene() override;
    void render_ui() override;
    void render_hud() override;
    void pre_draw() override;
    void post_draw() override;
};

