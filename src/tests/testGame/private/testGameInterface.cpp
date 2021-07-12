
#include "testGameInterface.h"

#include "assets/asset_material.h"
#include "assets/asset_mesh.h"
#include "assets/asset_mesh_data.h"
#include "assets/asset_shader.h"
#include "camera_basic_controller.h"
#include "imgui.h"
#include "ios/mesh_importer.h"
#include "ios/scene_importer.h"
#include "scene/node_camera.h"
#include "scene/node_mesh.h"
#include "ui/window/window_base.h"
#include "ui/window/windows/content_browser.h"
#include "ui/window/windows/profiler.h"

PlayerController* TestGameInterface::get_controller()
{
    return nullptr;
}

void TestGameInterface::load_resources()
{
    root_scene = std::make_unique<Scene>(get_asset_manager());


    // Create shaders
    const TAssetPtr<Shader> vertex_shader   = get_asset_manager()->create<Shader>("test_vertex_shader", "data/test.vs.glsl", EShaderStage::VertexShader);
    const TAssetPtr<Shader> fragment_shader = get_asset_manager()->create<Shader>("test_fragment_shader", "data/test.fs.glsl", EShaderStage::FragmentShader);

    // create material parameters
    const ShaderStageData vertex_stage{
        .shader         = vertex_shader,
        .uniform_buffer  = {root_scene->get_scene_uniform_buffer()},
        .storage_buffers = {root_scene->get_model_ssbo()},
    };
    const ShaderStageData fragment_stage{
        .shader         = fragment_shader,
        .uniform_buffer = {root_scene->get_scene_uniform_buffer()},
    };

    // create material
    const TAssetPtr<Material> material = get_asset_manager()->create<Material>("test_material", vertex_stage, fragment_stage);


    auto camera = root_scene->add_node<Camera>();

    root_scene->set_camera(camera);

    // Create mesh data
    SceneImporter scene_importer(get_asset_manager());
    const auto imported_node = scene_importer.import_file("data/sponza.glb", "sponza_elem", root_scene.get());

    controller = std::make_unique<CameraBasicController>(camera, get_input_manager());
}

void TestGameInterface::pre_initialize()
{
}

void TestGameInterface::pre_shutdown()
{
}

void TestGameInterface::unload_resources()
{
}

void TestGameInterface::render_scene(RenderContext render_context)
{
    if (auto mat = TAssetPtr<Material>(get_asset_manager(), "test_material"))
        mat->update_descriptor_sets(render_context.image_index);
    else
        LOG_ERROR("default material is not valid");
    root_scene->render_scene(render_context);
}

void TestGameInterface::render_ui()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("file"))
        {
            if (ImGui::MenuItem("quit"))
                close();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("misc"))
        {
            if (ImGui::MenuItem("demo window"))
                new DemoWindow(this, "demo window");
            if (ImGui::MenuItem("profiler"))
                new ProfilerWindow(this, "profiler");
            if (ImGui::MenuItem("content browser"))
                new ContentBrowser(this, "content browser");
            ImGui::EndMenu();
        }
        ImGui::Text("%lf fps", 1.0 / get_delta_second());
        ImGui::EndMainMenuBar();
    }
}

void TestGameInterface::render_hud()
{
}

void TestGameInterface::pre_draw()
{
    root_scene->tick(0);
    TAssetPtr<Material> material(get_asset_manager(), "test_material");
}

void TestGameInterface::post_draw()
{
}