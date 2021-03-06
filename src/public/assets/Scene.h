#pragma once
#include <filesystem>

#include "assetBase.h"


class SceneImporter;
class Node;

class Scene : public AssetBase
{
public:

	Scene(const std::filesystem::path& asset_path);

	virtual ~Scene();

	[[nodiscard]] Node* get_root_node() const { return root_node; }

private:
	std::shared_ptr<SceneImporter> importer;


	Node* root_node;
};