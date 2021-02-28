#pragma once
#include <memory>
#include <string>
#include <vector>


#include "engine/noncopyable.h"

class Window;

class AssetRef
{
public:
	AssetRef(const size_t id_val) : id(id_val) {}
	explicit AssetRef(const std::string& name);
	AssetRef(const char* name) : AssetRef(std::string(name)) {}

	size_t operator()() const { return id; }
	std::string to_string() const;
private:
	const size_t id;
#if _DEBUG
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
	~GraphicResourceManager()
	{
		for (const auto& resource : resources)
		{
			delete resource;
		}
	}

	void register_resource(GraphicResource* resource) { resources.push_back(resource); }

private:

	std::vector<GraphicResource*> resources;

};