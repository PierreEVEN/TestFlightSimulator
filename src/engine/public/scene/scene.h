#pragma once

#include "assets/asset_ptr.h"
#include "rendering/window.h"

#include "assets/asset_uniform_buffer.h"
#include <cpputils/logger.hpp>

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class AssetManager;
class Camera;
class UniformBuffer;
class Node;
class PrimitiveNode;
class RenderProxy;

struct CameraData
{
    glm::mat4 world_projection = glm::mat4(1.0);
    glm::mat4 view_matrix      = glm::mat4(1.0);
    glm::vec3 camera_location  = glm::vec3(0, 0, 0);
};

class Scene
{
    friend class PrimitiveNode;

  public:
    Scene(AssetManager* asset_manager);

    void tick(const double delta_second);
    void render_scene(RenderContext render_context);

    template <typename Node_T, typename... Args_T> std::shared_ptr<Node_T> add_node(Args_T&&... arguments)
    {
        Node_T* node_storage       = static_cast<Node_T*>(std::malloc(sizeof(Node_T)));
        node_storage->render_scene = this;

        std::shared_ptr<Node_T> node_ptr(node_storage);

        if (std::is_base_of<PrimitiveNode, Node_T>::value)
        {
            rendered_nodes.emplace_back(std::dynamic_pointer_cast<PrimitiveNode>(node_ptr));
        }

        new (node_storage) Node_T(std::forward<Args_T>(arguments)...);

        if (!node_storage->render_scene)
        {
            LOG_ERROR("don't call Node() constructor in children class : %s", typeid(Node_T).name());
        }

        scene_nodes.emplace_back(std::dynamic_pointer_cast<Node>(node_ptr));

        return node_ptr;
    }

    void set_camera(std::shared_ptr<Camera> new_camera);

    [[nodiscard]] TAssetPtr<UniformBuffer> get_scene_uniform_buffer() const
    {
        return camera_uniform_buffer;
    }

    [[nodiscard]] glm::dmat4 make_projection_matrix(const RenderContext& render_context) const;

  private:
    TAssetPtr<UniformBuffer> camera_uniform_buffer = nullptr;
    std::shared_ptr<Camera>  enabled_camera        = nullptr;

    std::vector<std::shared_ptr<Node>>          scene_nodes;
    std::vector<std::shared_ptr<PrimitiveNode>> rendered_nodes;
};
