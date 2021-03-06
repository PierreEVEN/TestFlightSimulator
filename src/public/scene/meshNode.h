#pragma once
#include "node.h"
#include "assets/assetPtr.h"


class StaticMesh;
class Shader;

class MeshNode : public Node
{
public:
	
	MeshNode(const TAssetPtr<StaticMesh>& in_mesh, const TAssetPtr<Shader>& in_material);

	void draw(VkCommandBuffer buffer, uint8_t image_index) override;

private:

	void bind_pipeline(VkCommandBuffer buffer, uint8_t image_index);
	void bind_mesh(VkCommandBuffer buffer);
	
	TAssetPtr<Shader> material;
	TAssetPtr<StaticMesh> mesh;
};
