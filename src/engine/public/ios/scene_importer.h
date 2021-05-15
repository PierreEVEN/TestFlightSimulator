#pragma once
#include <filesystem>
#include <unordered_map>



#include "assets/assetPtr.h"
#include "assets/graphicResource.h"

namespace Assimp {
class Importer;
}

class Texture2d;
class Shader;

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

	[[nodiscard]] Node* get_root_node() const { return root_node; }
private:

	TAssetPtr<Texture2d> process_texture(struct aiTexture* texture, size_t id);
	TAssetPtr<Shader> process_material(struct aiMaterial* material, size_t id);
	TAssetPtr<StaticMesh> process_mesh(struct aiMesh* mesh, size_t id);
	
	Node* process_node(aiNode* ai_node, Node* parent);
	Node* create_node(aiNode* context, Node* parent);
	
	Node* root_node = nullptr;
	Window* window_context = nullptr;

	std::string object_name;
	std::vector<TAssetPtr<Texture2d>> texture_refs;
	std::vector<TAssetPtr<Shader>> material_refs;
	std::vector<TAssetPtr<StaticMesh>> meshes_refs;
	
	std::unique_ptr<Assimp::Importer> importer;
};