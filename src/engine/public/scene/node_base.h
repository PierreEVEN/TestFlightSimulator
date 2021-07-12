#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include <memory>

namespace glm
{
typedef glm::vec<3, double, highp> dvec3;
}

class Scene;

class Node
{
    friend class Scene;

  public:
    Node()
    {
    }
    virtual ~Node() = default;

    [[nodiscard]] Scene* get_render_scene() const
    {
        return render_scene;
    }

    virtual void tick(const double delta_second)
    {
    }

    [[nodiscard]] const glm::dvec3& get_world_position() const
    {
        return world_position;
    }
    [[nodiscard]] const glm::dquat& get_world_rotation() const
    {
        return world_rotation;
    }
    [[nodiscard]] const glm::dvec3& get_world_scale() const
    {
        return world_scale;
    }

    [[nodiscard]] const glm::dvec3& get_relative_position() const
    {
        return rel_position;
    }
    [[nodiscard]] const glm::dquat& get_relative_rotation() const
    {
        return rel_rotation;
    }
    [[nodiscard]] const glm::dvec3& get_relative_scale() const
    {
        return rel_scale;
    }

    [[nodiscard]] const glm::dmat4& get_relative_transform() const
    {
        return rel_transform;
    }

    [[nodiscard]] const glm::dmat4& get_world_transform() const
    {
        return world_transform;
    }

    [[nodiscard]] glm::dvec3 get_forward_vector() const
    {
        return get_world_rotation() * glm::dvec3(1, 0, 0);
    }

    [[nodiscard]] glm::dvec3 get_right_vector() const
    {
        return get_world_rotation() * glm::dvec3(0, 1, 0);
    }

    [[nodiscard]] glm::dvec3 get_up_vector() const
    {
        return get_world_rotation() * glm::dvec3(0, 0, 1);
    }

    void set_relative_position(const glm::dvec3& in_position)
    {
        rel_position = in_position;
        recompute_transform();
    }

    void set_relative_rotation(const glm::dquat& in_rotation)
    {
        rel_rotation = in_rotation;
        recompute_transform();
    }

    void set_relative_scale(const glm::dvec3& in_scale)
    {
        rel_scale = in_scale;
        recompute_transform();
    }

    virtual void attach_to(const std::shared_ptr<Node>& new_parent_node, bool b_keep_world_transform = false);
    virtual void detach(bool b_keep_world_transform);

  protected:
  private:
    [[nodiscard]] bool ensure_node_can_be_attached(const Node* in_node) const;
    [[nodiscard]] bool is_node_in_hierarchy(const Node* in_node) const;

    void recompute_transform();

    void initialize_internal(Scene* in_scene, Node* in_parent);

    glm::dvec3         rel_position    = glm::dvec3(0.0);
    glm::dquat         rel_rotation    = glm::dquat();
    glm::dvec3         rel_scale       = glm::dvec3(1.0);
    glm::dmat4         rel_transform   = glm::dmat4(1.0);
    glm::dvec3         world_position    = glm::dvec3(0.0);
    glm::dquat         world_rotation    = glm::dquat();
    glm::dvec3         world_scale       = glm::dvec3(1.0);
    glm::dmat4         world_transform = glm::dmat4(1.0);

    Node*              parent          = nullptr;
    std::vector<Node*> children        = {};

    // Initialized in Scene constructor
    Scene* render_scene;
};