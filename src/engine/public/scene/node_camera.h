#pragma once
#include "node_base.h"

class Camera : public Node
{
public:

    Camera() : Node() {}

    [[nodiscard]] glm::dmat4 get_look_at_matrix() const
    {
        return glm::lookAt(get_world_position(), get_world_position() + get_forward_vector(), get_world_up());
    }

    [[nodiscard]] float get_field_of_view() const
    {
        return field_of_view;
    }

    [[nodiscard]] float get_near_clip_plane() const
    {
        return near_clip_plane;
    }

    [[nodiscard]] float get_far_clip_plane() const
    {
        return far_clip_plane;
    }

    [[nodiscard]] glm::dvec3 get_world_up() const
    {
        return glm::dvec3(0, 0, -1);
    }
  private:

    float      near_clip_plane = 0.1f;
    float      far_clip_plane  = 1000000.f;
    float      field_of_view   = 45.f;
};
