

#include "assets/Scene.h"

#include "ios/scene_importer.h"

Scene::Scene(const std::filesystem::path& asset_path)
{
	importer = std::make_shared<SceneImporter>(get_context(), asset_path, get_id().to_string());
	root_node = importer->get_root_node();
}

Scene::~Scene()
{
	
}
