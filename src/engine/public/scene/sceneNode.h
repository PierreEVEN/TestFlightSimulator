#pragma once
#include "assets/assetPtr.h"
#include "node.h"

class Scene;

class SceneNode : public Node
{
  public:
    SceneNode(TAssetPtr<Scene> in_scene);

    void draw(VkCommandBuffer buffer, uint8_t image_index) override;

  private:
    TAssetPtr<Scene> scene;
};
