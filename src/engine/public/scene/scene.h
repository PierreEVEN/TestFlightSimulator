#pragma once

#include "rendering/window.h"

#include <cpputils/logger.hpp>

#include <memory>
#include <vector>

class Node;
class PrimitiveNode;
class RenderProxy;

class Scene
{
    friend class PrimitiveNode;

  public:
    void tick(const double delta_second);
    void render_scene(RenderContext render_context);

    template <typename Node_T, typename... Args_T> Node_T* add_node(Args_T&&... arguments)
    {
        Node_T* node_storage       = static_cast<Node_T*>(std::malloc(sizeof(Node_T)));
        node_storage->render_scene = this;

        if (std::is_base_of<PrimitiveNode, Node_T>::value)
        {
            rendered_nodes.emplace_back(node_storage);
        }

        new (node_storage) Node_T(std::forward<Args_T>(arguments)...);

        scene_nodes.emplace_back(node_storage);

        return node_storage;
    }

  private:
    std::vector<std::shared_ptr<Node>> scene_nodes;
    std::vector<std::shared_ptr<PrimitiveNode>> rendered_nodes;
};
