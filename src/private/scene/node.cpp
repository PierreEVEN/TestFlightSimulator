

#include "scene/node.h"

#include "ios/logger.h"

Node::Node(Node* in_parent, const glm::dvec3& in_position, const glm::dquat& in_rotation,
           const glm::dvec3& in_scale)
	: position(in_position), rotation(in_rotation), scale(in_scale)
{
	if (in_parent) attach_to(in_parent, false);
}

void Node::attach_to(Node* target, bool keep_world_transform)
{
	if (!target)
	{
		logger_warning("attempting to attach node to null parent");
		return;
	}
	
	parent = target;
	parent->children.push_back(this);
}

void Node::detach(bool keep_world_transform)
{
	if (parent) {
		parent->children.erase(std::find(parent->children.begin(), parent->children.end(), this));
	}
	parent = nullptr;
}
