#pragma once
#include <filesystem>
#include <unordered_map>
#include <assimp/Importer.hpp>


#include "assets/GraphicResource.h"

namespace job_system {
	class IJobTask;
}

class AssetId;
class StaticMesh;
class Window;
class Node;
struct aiNode;

class SceneImporter final
{
public:
	SceneImporter(Window* context, const std::filesystem::path& source_file, const std::string& desired_asset_name = "");
	~SceneImporter();
private:

	std::shared_ptr<AssetId> process_texture(struct aiTexture* texture, size_t id);
	std::shared_ptr<AssetId> process_material(struct aiMaterial* material, size_t id);
	std::shared_ptr<AssetId> process_mesh(struct aiMesh* mesh, size_t id);
	
	void process_node(aiNode* ai_node, Node* parent);
	Node* create_node(aiNode* context, Node* parent);
	
	Node* root_node = nullptr;
	Window* window_context = nullptr;

	std::string object_name;
	std::unordered_map<size_t, std::shared_ptr<AssetId>> texture_refs;
	std::unordered_map<size_t, std::shared_ptr<AssetId>> material_refs;
	std::unordered_map<size_t, std::shared_ptr<AssetId>> meshes_refs;
	
	std::shared_ptr<job_system::IJobTask> creation_job;

	Assimp::Importer importer;
};