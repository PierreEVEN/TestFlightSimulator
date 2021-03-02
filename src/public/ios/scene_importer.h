#pragma once
#include <filesystem>
#include <unordered_map>

#include "assets/GraphicResource.h"

class StaticMesh;
class Window;
class Node;
struct aiNode;

class SceneImporter
{
public:
	SceneImporter(Window* context, const std::filesystem::path& source_file);

private:

	std::shared_ptr<AssetRef> process_texture(struct aiTexture* texture);
	std::shared_ptr<AssetRef> process_material(struct aiMaterial* material);
	std::shared_ptr<AssetRef> process_mesh(struct aiMesh* mesh);
	
	void process_node(aiNode* ai_node, Node* parent);
	Node* create_node(aiNode* context, Node* parent);
	
	Node* root_node = nullptr;
	Window* window_context = nullptr;

	std::string object_name;
	std::unordered_map<size_t, std::shared_ptr<AssetRef>> texture_refs;
	std::unordered_map<size_t, std::shared_ptr<AssetRef>> material_refs;
	std::unordered_map<size_t, std::shared_ptr<AssetRef>> meshes_refs;
};