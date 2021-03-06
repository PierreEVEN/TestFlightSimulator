

#include "scene/sceneNode.h"

#include "assets/Scene.h"

SceneNode::SceneNode(TAssetPtr<Scene> in_scene)
	: Node(), scene(in_scene)
{	
}

void SceneNode::draw(VkCommandBuffer buffer, uint8_t image_index)
{
	scene->get_root_node()->draw(buffer, image_index);
}
