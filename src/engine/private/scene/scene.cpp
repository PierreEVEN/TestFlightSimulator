

#include "scene/scene.h"

#include "scene/node_primitive.h"

void Scene::tick(const double delta_second)
{
    for (const auto& component : scene_nodes)
        component->tick(delta_second);
}

void Scene::render_scene(RenderContext render_context)
{
    for (const auto& component : rendered_nodes)
        component->render(render_context);

}
