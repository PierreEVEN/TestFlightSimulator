
#include "testGameInterface.h"

#include "assets/Scene.h"
#include "assets/shader.h"
#include "assets/texture2d.h"
#include "ui/window/windowBase.h"
#include "ui/window/windows/contentBrowser.h"
#include "ui/window/windows/profiler.h"
PlayerController* TestGameInterface::get_controller() { return nullptr; }

void TestGameInterface::load_resources()
{
    get_asset_manager()->create<Texture2d>("default-texture", "data/DefaultTexture.png");
    get_asset_manager()->create<Shader>("shader_Test", "data/test.vs.glsl", "data/test.fs.glsl");
    get_asset_manager()->create<Scene>("F-16", "data/F-16_b.glb");
    for (int i = 0; i < 50; ++i)
    {
        get_asset_manager()->create<Texture2d>(AssetId("de2fault-texture" + std::to_string(i)), "data/DefaultTexture.png");
        get_asset_manager()->create<Shader>(AssetId("shad2er_Test"+ std::to_string(i)), "data/test.vs.glsl", "data/test.fs.glsl");
        get_asset_manager()->create<Scene>(AssetId("F-162" + std::to_string(i)), "data/F-16_b.glb");
    }
}

void TestGameInterface::pre_initialize() {}

void TestGameInterface::pre_shutdown() {}

void TestGameInterface::unload_resources() {}

void TestGameInterface::render_scene() {}

void TestGameInterface::render_ui()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("misc"))
        {            
            if (ImGui::MenuItem("demo window")) new DemoWindow(this, "demo window");
            if (ImGui::MenuItem("profiler")) new ProfilerWindow(this, "profiler");
            if (ImGui::MenuItem("content browser")) new ContentBrowser(this, "content browser");
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void TestGameInterface::render_hud()
{
    ImGui::Text("test");
    ImGui::Text("test");
}

void TestGameInterface::pre_draw() {}

void TestGameInterface::post_draw() {}
