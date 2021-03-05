#pragma once
#include "node.h"
#include "assets/assetPtr.h"


class StaticMesh;
class Shader;

class MeshNode : public Node
{
public:

	using Node::Node;

private:

	TAssetPtr<Shader> material;
	TAssetPtr<StaticMesh> mesh;
};
