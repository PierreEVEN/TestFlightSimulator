

#include "assets/Scene.h"


#include "jobSystem/job_system.h"
#include "ios/scene_importer.h"
#include "rendering/window.h"

Scene::Scene(const std::filesystem::path& asset_path)
{
	creation_task = job_system::new_job([&, asset_path] {
		importer = std::make_shared<SceneImporter>(get_context(), asset_path, get_id().to_string());
		root_node = importer->get_root_node();

		get_context()->TEMP_NODE = root_node;
		
		});
}

Scene::~Scene()
{
	creation_task->wait();
}
