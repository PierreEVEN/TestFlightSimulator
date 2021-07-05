#pragma once

#include <cpputils/logger.hpp>

#include <memory>
#include <vector>

class Node;
class RenderProxy;

class Scene
{
    friend class PrimitiveNode;

  public:
    void tick(const double delta_second);

    template <typename Node_T, typename... Args_T> Node_T* add_node(Args_T&&... arguments)
    {
        Node_T* node_storage       = static_cast<Node_T*>(std::malloc(sizeof(Node_T)));
        node_storage->render_scene = this;
        new (node_storage) Node_T(std::forward<Args_T>(arguments)...);

        scene_nodes.emplace_back(node_storage);

        return node_storage;
    }

  private:
    std::vector<std::shared_ptr<Node>> scene_nodes;
};
