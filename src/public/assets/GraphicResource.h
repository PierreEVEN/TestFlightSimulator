#pragma once
#include <memory>
#include <mutex>
#include <string>
#include <vector>


#include "engine/noncopyable.h"

class GraphicResource;
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
	size_t id;
#ifdef _DEBUG
	std::string asset_name;
#endif
};




class GraphicResourceManager final
{
public:

	GraphicResourceManager() = default;

	static void register_resource_static(Window* context, GraphicResource* resource);
	void register_resource(GraphicResource* resource) {
		std::lock_guard<std::mutex> lock(resource_manager_lock);
		resources.push_back(resource);
	}

	void clean();

private:
	std::mutex resource_manager_lock;
	std::vector<GraphicResource*> resources;
};

class GraphicResource : public NonCopyable {
protected:

	GraphicResource(Window* context, const AssetRef& asset_reference);

public:
	virtual ~GraphicResource() = default;

	template<class Resource, typename ... Args>
	static Resource* create(Window* context, Args... arguments)
	{
		Resource* resource = new Resource(context, std::forward<Args>(arguments)...);
		GraphicResourceManager::register_resource_static(context, resource);
		return resource;
	}
protected:
	Window* window_context;

private:
	const AssetRef asset_ref;
};