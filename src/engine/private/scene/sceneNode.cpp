

#include "scene/sceneNode.h"

#include "assets/SceneAsset.h"

SceneNode::SceneNode(TAssetPtr<SceneAsset> in_scene)
	: Node(), scene(in_scene)
{	
}

void SceneNode::draw(VkCommandBuffer buffer, uint8_t image_index)
{
	Node::draw(buffer, image_index);
	scene->get_root_node()->draw(buffer, image_index);
}
