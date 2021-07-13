#pragma once

#include "assets/asset_ptr.h"
#include "rendering/window.h"
#include "scene_proxy.h"

#include "assets/asset_shader_buffer.h"
#include <cpputils/logger.hpp>

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class AssetManager;
class Camera;
class ShaderBuffer;
class Node;
class PrimitiveNode;

struct CameraData
{
    glm::mat4 world_projection = glm::mat4(1.0);
    glm::mat4 view_matrix      = glm::mat4(1.0);
    glm::vec3 camera_location  = glm::vec3(0, 0, 0);
};

#define ENTITY_SIZE 256

template <const size_t type_size> using Entity = uint8_t[type_size];

class RenderProxy
{
    int add_entity()
    {
        return 0; // entity_handle
    }

    void remove_entity(int entity_handle)
    {
    }

    Entity<256>                  entity_data[64];
    bool                         data_update_status[64];
    size_t                       entity_count = 64;
    std::unordered_map<int, int> index_map;
};

class RenderProxyECS
{
  public:
  private:
    RenderProxy entities    = {};
    VkBuffer    ssbo_handle = VK_NULL_HANDLE;
};

class Scene
{
    friend class PrimitiveNode;

  public:
    Scene(AssetManager* asset_manager);

    void tick(const double delta_second);
    void render_scene(RenderContext& render_context);

    template <typename Node_T, typename... Args_T> std::shared_ptr<Node_T> add_node(Args_T&&... arguments)
    {
        Node_T* node_storage       = static_cast<Node_T*>(std::malloc(sizeof(Node_T)));
        node_storage->render_scene = this;

        std::shared_ptr<Node_T> node_ptr(node_storage);
        
        new (node_storage) Node_T(std::forward<Args_T>(arguments)...);

        if (!node_storage->render_scene)
        {
            LOG_ERROR("don't call Node() constructor in children class : %s", typeid(Node_T).name());
        }

        scene_nodes.emplace_back(std::dynamic_pointer_cast<Node>(node_ptr));

        return node_ptr;
    }

    void set_camera(std::shared_ptr<Camera> new_camera);

    [[nodiscard]] TAssetPtr<ShaderBuffer> get_scene_uniform_buffer() const
    {
        return camera_uniform_buffer;
    }

    [[nodiscard]] TAssetPtr<ShaderBuffer> get_model_ssbo() const
    {
        return global_model_ssbo;
    }

    [[nodiscard]] glm::dmat4 make_projection_matrix(const RenderContext& render_context) const;

    [[nodiscard]] SceneProxy& get_scene_proxy()
    {
        return scene_proxy;
    }

  private:
    TAssetPtr<ShaderBuffer> camera_uniform_buffer = nullptr;
    TAssetPtr<ShaderBuffer> global_model_ssbo     = nullptr;
    std::shared_ptr<Camera> enabled_camera        = nullptr;

    std::vector<std::shared_ptr<Node>>          scene_nodes;
    SceneProxy                                  scene_proxy;
};
