#pragma once
#include <filesystem>

#include "assetBase.h"
#include "jobSystem/job.h"


namespace job_system {
	class IJobTask;
}

class SceneImporter;
class Node;

class SceneAsset : public AssetBase
{
public:

	SceneAsset(const std::filesystem::path& asset_path);

	virtual ~SceneAsset();

	[[nodiscard]] Node* get_root_node() const { return root_node; }

	bool try_load() override { return creation_task->is_complete(); }

private:
	std::shared_ptr<SceneImporter> importer;

	std::shared_ptr<job_system::IJobTask> creation_task;
	
	Node* root_node;
};