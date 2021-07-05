

#include "scene/scene.h"

#include "scene/node_primitive.h"

void Scene::tick(const double delta_second)
{
    for (const auto& component : scene_nodes)
        component->tick(delta_second);
}