
#include "testGameInterface.h"

#include "camera_basic_controller.h"
#include "assets/asset_material.h"
#include "assets/asset_mesh.h"
#include "assets/asset_mesh_data.h"
#include "assets/asset_shader.h"
#include "assets/asset_uniform_buffer.h"
#include "imgui.h"
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
    get_input_manager()->add_input(InputAction("test", {GLFW_KEY_ESCAPE, GLFW_KEY_A}));
    get_input_manager()->get_input("test")->press_event.Add(this, &TestGameInterface::truc_press);
    get_input_manager()->get_input("test")->pressed_event.Add(this, &TestGameInterface::truc_pressed);
    get_input_manager()->get_input("test")->released_event.Add(this, &TestGameInterface::truc_released);

    root_scene = std::make_unique<Scene>(get_asset_manager());

    // Create mesh data
    const TAssetPtr<MeshData> mesh_data = get_asset_manager()->create<MeshData>(
        "test_mesh_data", std::vector<Vertex>{{.pos = glm::vec3(10, -5, -5)}, {.pos = glm::vec3(10, -5, 5)}, {.pos = glm::vec3(10, 5, 5)}, {.pos = glm::vec3(10, 5, -5)}}, std::vector<uint32_t>{0, 1, 2, 0, 3, 2});

    // Create shaders
    const TAssetPtr<Shader> vertex_shader   = get_asset_manager()->create<Shader>("test_vertex_shader", "data/test.vs.glsl", EShaderStage::VertexShader);
    const TAssetPtr<Shader> fragment_shader = get_asset_manager()->create<Shader>("test_fragment_shader", "data/test.fs.glsl", EShaderStage::FragmentShader);

    // create material parameters
    const ShaderStageData vertex_stage{
        .shader         = vertex_shader,
        .uniform_buffer = {root_scene->get_scene_uniform_buffer()},
    };
    const ShaderStageData fragment_stage{
        .shader         = fragment_shader,
        .uniform_buffer = {root_scene->get_scene_uniform_buffer()},
    };

    // create material
    const TAssetPtr<Material> material = get_asset_manager()->create<Material>("test_material", vertex_stage, fragment_stage, std::make_shared<PushConstant>(glm::mat4(1.0)));

    // assemble mesh
    const TAssetPtr<Mesh> mesh = get_asset_manager()->create<Mesh>("test_mesh", mesh_data, material);

    // add test scene component to scene
    auto node_1 = root_scene->add_node<MeshNode>(mesh, material);
    auto node_2 = root_scene->add_node<MeshNode>(mesh, material);
    auto camera = root_scene->add_node<Camera>();
    node_2->set_relative_position(glm::vec3(1, 0, 0));

    root_scene->set_camera(camera);

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
    TAssetPtr<Material>(this, "test_material")->update_descriptor_sets(render_context.image_index);
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

void TestGameInterface::truc_pressed(const InputAction&)
{
    LOG_WARNING("pressed");
}

void TestGameInterface::truc_released(const InputAction&)
{
    LOG_WARNING("released");
}

void TestGameInterface::truc_press(const InputAction&)
{
    LOG_WARNING("press");
}