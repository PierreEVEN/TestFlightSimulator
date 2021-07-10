
#include "testGameInterface.h"

#include "assets/asset_material.h"
#include "assets/asset_mesh.h"
#include "assets/asset_mesh_data.h"
#include "assets/asset_shader.h"
#include "assets/asset_uniform_buffer.h"
#include "imgui.h"
#include "scene/node_mesh.h"
#include "scene/node_camera.h"
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

    // Create mesh data
    const TAssetPtr<MeshData> mesh_data = get_asset_manager()->create<MeshData>(
        "test_mesh_data", std::vector<Vertex>{{.pos = glm::vec3(-10, -10, 1)}, {.pos = glm::vec3(10, -10, 1)}, {.pos = glm::vec3(10, 10, 1)}, {.pos = glm::vec3(-10, 10, 1)}}, std::vector<uint32_t>{0, 1, 2, 0, 3, 2});

    // Create shaders
    const TAssetPtr<Shader> vertex_shader   = get_asset_manager()->create<Shader>("test_vertex_shader", "data/test.vs.glsl", EShaderStage::VertexShader);
    const TAssetPtr<Shader> fragment_shader = get_asset_manager()->create<Shader>("test_fragment_shader", "data/test.fs.glsl", EShaderStage::FragmentShader);

    // create material parameters
    const ShaderStageData          vertex_stage{
        .shader         = vertex_shader,
        .uniform_buffer = {root_scene->get_scene_uniform_buffer()},
    };
    const ShaderStageData fragment_stage{
        .shader         = fragment_shader,
        .uniform_buffer = {root_scene->get_scene_uniform_buffer()},
    };

    // create material
    glm::mat4 model_mat(1.0);
    model_mat[0][3]                    = 1;
    const TAssetPtr<Material> material = get_asset_manager()->create<Material>("test_material", vertex_stage, fragment_stage, std::make_shared<PushConstant>(model_mat));

    // assemble mesh
    const TAssetPtr<Mesh> mesh = get_asset_manager()->create<Mesh>("test_mesh", mesh_data, material);

    // add test scene component to scene
    root_scene->add_node<MeshNode>(mesh, material);
    root_scene->set_camera(root_scene->add_node<Camera>());
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
    TAssetPtr<Material> material(this, "test_material");
}

void TestGameInterface::post_draw()
{
}