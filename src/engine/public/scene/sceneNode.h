#pragma once
#include "assets/assetPtr.h"
#include "node.h"

class SceneAsset;

class SceneNode : public Node
{
  public:
    SceneNode(TAssetPtr<SceneAsset> in_scene);

    void draw(VkCommandBuffer buffer, uint8_t image_index) override;

  private:
    TAssetPtr<SceneAsset> scene;
};
