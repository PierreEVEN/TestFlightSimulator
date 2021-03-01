#pragma once
#include <memory>
#include <mutex>
#include <string>
#include <vector>


#include "engine/noncopyable.h"

class Window;

class AssetRef
{
public:
	AssetRef(const size_t id_val);
	explicit AssetRef(const std::string& name);
	AssetRef(const char* name) : AssetRef(std::string(name)) {}

	size_t operator()() const { return id; }
	std::string to_string() const;
private:
	const size_t id;
#ifdef _DEBUG
	std::string asset_name;
#endif
};


class GraphicResource : public NonCopyable {
public:
	GraphicResource(Window* context, const AssetRef& asset_reference);
	virtual ~GraphicResource() = default;
	
private:
	const AssetRef asset_ref;
};

class GraphicResourceManager final
{
public:
	GraphicResourceManager() = default;

	void register_resource(GraphicResource* resource) {
		std::lock_guard<std::mutex> lock(resource_manager_lock);
		resources.push_back(resource);
	}

	void clean();

private:
	std::mutex resource_manager_lock;
	std::vector<GraphicResource*> resources;
};