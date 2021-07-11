

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
        CameraData camera_data = {
            .world_projection = make_projection_matrix(render_context),
            .view_matrix      = enabled_camera->get_look_at_matrix(),
            .camera_location  = enabled_camera->get_world_position(),
        };
        camera_uniform_buffer->set_data(camera_data);
    }
    else
    {
        LOG_WARNING("no default camera enabled for this scene");
        return;
    }
    for (const auto& component : rendered_nodes)
    {
        component->render(render_context);
    }
}

void Scene::set_camera(std::shared_ptr<Camera> new_camera)
{
    if (new_camera->get_render_scene() != this)
    {
        LOG_ERROR("cannot set scene's default camera if the camera is not owned by the scene : %x", this);
    }
    else
        enabled_camera = std::move(new_camera);
}

glm::dmat4 Scene::make_projection_matrix(const RenderContext& render_context) const
{
    if (!enabled_camera)
        return glm::dmat4(1.0);
    return glm::perspective<double>(enabled_camera->get_field_of_view(), render_context.res_x / static_cast<double>(render_context.res_y), enabled_camera->get_near_clip_plane(), enabled_camera->get_far_clip_plane());
}
