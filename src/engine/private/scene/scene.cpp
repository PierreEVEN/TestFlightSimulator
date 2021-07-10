

#include "scene/scene.h"

#include "assets/asset_base.h"
#include "scene/node_camera.h"
#include "scene/node_primitive.h"

Scene::Scene(AssetManager* asset_manager)
{
    camera_uniform_buffer = asset_manager->create<UniformBuffer>("global_camera_uniform_buffer", "GlobalCameraUniformBuffer", CameraData{});
}

void Scene::tick(const double delta_second)
{
    for (const auto& component : scene_nodes)
        component->tick(delta_second);
}

void Scene::render_scene(RenderContext render_context)
{
    if (enabled_camera)
    {
        const CameraData camera_data = {
            .world_projection = glm::mat4(),
            .view_matrix      = enabled_camera->get_look_at_matrix(),
            .camera_location  = enabled_camera->get_world_position(),
        };
        camera_uniform_buffer->set_data(camera_data);
    }
    for (const auto& component : rendered_nodes)
    {
        component->render(render_context);
    }
}
