
#include "testGameInterface.h"

#include "assets/asset_material.h"
#include "assets/asset_mesh_data.h"
#include "assets/asset_shader.h"
#include "camera_basic_controller.h"
#include "imgui.h"
#include "ios/mesh_importer.h"
#include "ios/scene_importer.h"
#include "scene/node_camera.h"
#include "scene/node_mesh.h"
#include "scene/scene_proxy.h"
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

    root_scene->get_scene_proxy().register_entity_type<MeshProxyData>([](MeshProxyData& entity, RenderContext& render_context) {
        

        if (render_context.last_used_material != entity.material)
        {
            render_context.last_used_material = entity.material;
            vkCmdBindDescriptorSets(render_context.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_context.last_used_material->get_pipeline_layout(), 0, 1,
                                    &render_context.last_used_material->get_descriptor_sets()[render_context.image_index],
                                    0,
                                    nullptr);
            vkCmdBindPipeline(render_context.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, entity.material->get_pipeline());
        }

        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(render_context.command_buffer, 0, 1, &entity.vertex_buffer, offsets);
        vkCmdBindIndexBuffer(render_context.command_buffer, entity.index_buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(render_context.command_buffer, static_cast<uint32_t>(entity.indice_count), 1, 0, 0, 0);
    });

    // Create shaders
    const TAssetPtr<Shader> vertex_shader   = AssetManager::get()->create<Shader>("test_vertex_shader", "data/test.vs.glsl", EShaderStage::VertexShader);
    const TAssetPtr<Shader> fragment_shader = AssetManager::get()->create<Shader>("test_fragment_shader", "data/test.fs.glsl", EShaderStage::FragmentShader);

    // create material parameters
    const ShaderStageData vertex_stage{
        .shader          = vertex_shader,
        .uniform_buffer  = {root_scene->get_scene_uniform_buffer()},
        .storage_buffers = {root_scene->get_model_ssbo()},
    };
    const ShaderStageData fragment_stage{
        .shader          = fragment_shader,
        .uniform_buffer  = {root_scene->get_scene_uniform_buffer()},
        .storage_buffers = {},
    };

    // create material
    TAssetPtr<Material> material = AssetManager::get()->create<Material>("test_material", vertex_stage, fragment_stage);
    auto camera = root_scene->add_node<Camera>();
    root_scene->set_camera(camera);

    // Create mesh data
    SceneImporter scene_importer;
    const auto    imported_node = scene_importer.import_file("data/sponza.glb", "sponza_elem", root_scene.get());
    
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
    if (auto mat = TAssetPtr<Material>("test_material"))
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
                new DemoWindow("demo window");
            if (ImGui::MenuItem("profiler"))
                new ProfilerWindow("profiler");
            if (ImGui::MenuItem("content browser"))
                new ContentBrowser("content browser");
            ImGui::EndMenu();
        }
        ImGui::Text("%d fps   ...   %lf ms", static_cast<int>(1.0 / get_delta_second()), get_delta_second() * 1000.0);
        ImGui::EndMainMenuBar();
    }
}

void TestGameInterface::render_hud()
{
}

void TestGameInterface::pre_draw()
{
    root_scene->tick(0);
    TAssetPtr<Material> material("test_material");
}

void TestGameInterface::post_draw()
{
}