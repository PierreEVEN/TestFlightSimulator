
#include "scene/node_base.h"

void Node::recompute_transform()
{
    relative_transform = glm::mat4();
    relative_transform = glm::translate(relative_transform, rotation * position);
    relative_transform = glm::scale(relative_transform, scale);

    // @TODO compute relative transformation
    world_transform = relative_transform;
}
