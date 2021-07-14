

#include "scene/scene.h"

#include "assets/asset_base.h"
#include "scene/node_camera.h"
#include "scene/node_primitive.h"

struct ModMatrix
{
    glm::mat4 a;
};
Scene::Scene()
{
    camera_uniform_buffer = AssetManager::get()->create<ShaderBuffer>("global_camera_uniform_buffer", "GlobalCameraUniformBuffer", CameraData{}, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    global_model_ssbo     = AssetManager::get()->create<ShaderBuffer>("global_object_buffer", "ObjectBuffer", sizeof(ModMatrix) * 100, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

void Scene::tick(const double delta_second)
{
    for (const auto& component : scene_nodes)
        component->tick(delta_second);
}

void Scene::render_scene(RenderContext& render_context)
{
    if (enabled_camera)
    {
        CameraData camera_data = {
            .world_projection = make_projection_matrix(render_context),
            .view_matrix      = enabled_camera->get_look_at_matrix(),
            .camera_location  = enabled_camera->get_world_position(),
        };
        camera_uniform_buffer->set_data(camera_data);

        std::vector<ModMatrix> matrix(100);
        for (int i = 0; i < 100; ++i)
        {
            matrix[i].a = glm::mat4(1.0);
        }

        global_model_ssbo->write_buffer(matrix.data(), matrix.size() * sizeof(ModMatrix), 0);
    }
    else
    {
        LOG_WARNING("no default camera enabled for this scene");
        return;
    }
    scene_proxy.render(render_context);
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
