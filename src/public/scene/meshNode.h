#pragma once
#include "node.h"
#include "assets/assetPtr.h"


class StaticMesh;
class Shader;

class MeshNode : public Node
{
public:

	using Node::Node;

	void draw(VkCommandBuffer buffer, uint8_t image_index) override;

private:

	void bind_pipeline(VkCommandBuffer buffer, uint8_t image_index);
	void bind_mesh(VkCommandBuffer buffer);
	
	TAssetPtr<Shader> material;
	TAssetPtr<StaticMesh> mesh;
};
