#pragma once
#include "node.h"
#include "assets/assetPtr.h"


class Scene;

class SceneNode : public Node
{
public:

	SceneNode(TAssetPtr<Scene> in_scene);
	
	void draw(VkCommandBuffer buffer, uint8_t image_index) override;

private:

	TAssetPtr<Scene> scene;

	
};
