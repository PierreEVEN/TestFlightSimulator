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
    Node() = default;

    void initialize_internal(Scene* in_scene, Node* in_parent);
    virtual ~Node() = default;

    [[nodiscard]] Scene* get_render_scene() const
    {
        return render_scene;
    }

    virtual void tick(const double delta_second)
    {
    }

  private:
    glm::dvec3         position = glm::dvec3();
    glm::dquat         rotation = glm::dquat();
    glm::dvec3         scale    = glm::dvec3();
    Node*              parent   = nullptr;
    std::vector<Node*> children;

    // Initialized in Scene constructor
    Scene* render_scene;
};