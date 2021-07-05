
#include "testGameInterface.h"

#include "assets/asset_material.h"
#include "assets/asset_mesh.h"
#include "assets/asset_mesh_data.h"
#include "assets/asset_shader.h"
#include "imgui.h"
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

    root_scene = std::make_unique<Scene>();

    const TAssetPtr<MeshData> mesh_data = get_asset_manager()->create<MeshData>(
        "test_mesh_data", std::vector<Vertex>{Vertex{.pos = glm::vec3(0, 0, 0)}, Vertex{.pos = glm::vec3(1, 0, 0)}, Vertex{.pos = glm::vec3(1, 1, 0)}, Vertex{.pos = glm::vec3(0, 1, 0)}},
        std::vector<uint32_t>{0, 1, 2, 0, 2, 3});

    const TAssetPtr<Shader>   vertex_shader   = get_asset_manager()->create<Shader>("test_vertex_shader", "data/test.vs.glsl", EShaderKind::VertexShader);
    const TAssetPtr<Shader>   fragment_shader = get_asset_manager()->create<Shader>("test_fragment_shader", "data/test.fs.glsl", EShaderKind::FragmentShader);
    const TAssetPtr<Material> material        = get_asset_manager()->create<Material>("test_material", vertex_shader, fragment_shader);
    const TAssetPtr<Mesh>     mesh            = get_asset_manager()->create<Mesh>("test_mesh", mesh_data, material); // done

    MeshNode* test_node = root_scene->add_node<MeshNode>(mesh, material);

    root_scene->tick(0);
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
    MeshNode test_node(TAssetPtr<Mesh>(this, "test_mesh"), nullptr);

    test_node.render(render_context.command_buffer, render_context.image_index);
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
}

void TestGameInterface::post_draw()
{
}
