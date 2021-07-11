#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace glm
{
typedef glm::vec<3, double, highp> dvec3;
}

class Scene;

class Node
{
    friend class Scene;

  public:
    Node() {}
    virtual ~Node() = default;

    [[nodiscard]] Scene* get_render_scene() const
    {
        return render_scene;
    }

    virtual void tick(const double delta_second)
    {
    }

    [[nodiscard]] glm::dvec3 get_world_position() const
    {
        return position;
    }

    [[nodiscard]] glm::dvec3 get_relative_position() const
    {
        return position;
    }

    [[nodiscard]] glm::dquat get_world_rotation() const
    {
        return rotation;
    }

    [[nodiscard]] glm::dquat get_relative_rotation() const
    {
        return rotation;
    }

    [[nodiscard]] glm::dvec3 get_relative_scale() const
    {
        return scale;
    }

    [[nodiscard]] glm::dmat4 get_relative_transform() const
    {
        return relative_transform;
    }

    [[nodiscard]] glm::dmat4 get_world_transform() const
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

    [[nodiscard]] glm::dvec3 get_world_up() const
    {
        return glm::dvec3(0, 0, 1);
    }
    
    void set_relative_position(const glm::dvec3& in_position)
    {
        position = in_position;
        recompute_transform();
    }

    void set_relative_rotation(const glm::dquat& in_rotation)
    {
        rotation = in_rotation;
        recompute_transform();
    }

    void set_relative_scale(const glm::dvec3& in_scale)
    {
        scale = in_scale;
        recompute_transform();
    }

  protected:
  private:

      void recompute_transform();

    void initialize_internal(Scene* in_scene, Node* in_parent);

    glm::dvec3         position           = glm::dvec3(0.0);
    glm::dquat         rotation           = glm::dquat();
    glm::dvec3         scale              = glm::dvec3(1.0);
    glm::dmat4         relative_transform = glm::dmat4(1.0);
    glm::dmat4         world_transform    = glm::dmat4(1.0);
    Node*              parent             = nullptr;
    std::vector<Node*> children           = {};

    // Initialized in Scene constructor
    Scene* render_scene;
};
