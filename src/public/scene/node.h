#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>


#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace glm {
	typedef glm::vec<3, double, highp> dvec3;
}

class Node
{
public:
	Node(Node* in_parent = nullptr, const glm::dvec3& in_position = glm::dvec3(0), const glm::dquat& in_rotation = glm::dquat(), const glm::dvec3& in_scale = glm::dvec3(1));

	void attach_to(Node* target, bool keep_world_transform = true);
	void detach(bool keep_world_transform = true);

	virtual void draw(VkCommandBuffer buffer, uint8_t image_index) {}

private:
	glm::dvec3 position;
	glm::dquat rotation;
	glm::dvec3 scale;
	Node* parent = nullptr;
	std::vector<Node*> children;
};