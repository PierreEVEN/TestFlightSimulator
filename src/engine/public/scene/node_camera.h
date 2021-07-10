#pragma once
#include "node_base.h"

class Camera : public Node
{
public:

    [[nodiscard]] glm::dmat4 get_look_at_matrix() const
    {
        return glm::lookAt(get_world_position(), get_world_position() + get_forward_vector(), get_world_up());
    }

private:

    float      near_clip_plane = 0.1f;
    float      far_clip_plane  = 1000000.f;
    float      field_of_view   = 45.f;
};
