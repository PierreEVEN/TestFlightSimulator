
#include "scene/node_base.h"

#include <cpputils/logger.hpp>
#include <algorithm>

void Node::attach_to(const std::shared_ptr<Node>& new_parent_node, const bool b_keep_world_transform)
{
    if (!ensure_node_can_be_attached(new_parent_node.get()))
    {
        LOG_WARNING("cannot attach_to component to this one");
        return;
    }

    new_parent_node->children.emplace_back(this);
    parent = new_parent_node.get();

    if (b_keep_world_transform)
    {
        //@TODO handle b_keep_world_transform
        LOG_FATAL("keep world transform is not handled yet");
    }
    else
    {
        recompute_transform();
    }
}

void Node::detach(bool b_keep_world_transform)
{
    if (b_keep_world_transform)
    {
        //@TODO handle b_keep_world_transform
        LOG_FATAL("keep world transform is not handled yet");
    }
    else
    {
        recompute_transform();
    }

    if (parent)
    {
        parent->children.erase(std::ranges::find(parent->children, this));
    }

    parent = nullptr;
}

bool Node::ensure_node_can_be_attached(const Node* in_node) const
{
    if (!in_node)
    {
        LOG_WARNING("invalid node");
    }

    if (in_node == this)
    {
        LOG_ERROR("cannot attach node to itself");
        return false;
    }

    if (in_node->render_scene != render_scene)
    {
        LOG_ERROR("cannot attach_to node to this one if it is not owned by the same scene");
        return false;
    }

    if (is_node_in_hierarchy(in_node))
    {
        LOG_ERROR("cannot attach_to node to this one : this node is already attached to the current hierarchy");
        return false;
    }

    if (in_node->is_node_in_hierarchy(this))
    {
        LOG_ERROR("cannot attach_to node to this one : this node is already attached to the current hierarchy.");
        return false;
    }

    return true;
}

bool Node::is_node_in_hierarchy(const Node* in_node) const
{
    if (in_node == this)
        return true;

    if (parent)
        return parent->is_node_in_hierarchy(in_node);

    return false;
}

void Node::recompute_transform()
{
    rel_transform = glm::translate(glm::dmat4(1.0), rel_rotation * rel_position);
    rel_transform = glm::scale(rel_transform, rel_scale);

    if (parent)
    {
        world_transform = rel_transform * parent->get_world_transform();
        world_position = parent->get_world_transform() * glm::dvec4(rel_position, 0.0);
        world_rotation = parent->get_world_rotation() * rel_rotation;
        world_scale    = rel_scale * parent->get_world_scale();
    }
    else
    {
        world_transform = rel_transform;
        world_position  = rel_position;
        world_rotation  = rel_rotation;
        world_scale     = rel_scale;
    }

    for (const auto& child : children)
    {
        child->recompute_transform();
    }
}
