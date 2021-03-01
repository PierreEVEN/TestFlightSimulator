

#include "assets/GraphicResource.h"

#include "rendering/window.h"

AssetRef::AssetRef(const size_t id_val)
	: id(id_val) {}

AssetRef::AssetRef(const std::string& name) : AssetRef(std::hash<std::string>{}(name))
{
#ifdef _DEBUG
		asset_name = name;
#endif
}

std::string AssetRef::to_string() const
{
#ifdef _DEBUG
	return asset_name;
#else
	return std::to_string(id);
#endif
}

GraphicResource::GraphicResource(Window* context, const AssetRef& asset_reference)
	: asset_ref(asset_reference) {}

void GraphicResourceManager::clean()
{
	logger_log("cleaning up graphic resources");
	std::lock_guard<std::mutex> lock(resource_manager_lock);
	for (GraphicResource* resource : resources)
	{
		delete resource;
	}
}

void GraphicResourceManager::register_resource_static(Window* context, GraphicResource* resource)
{
	context->get_resource_manager().register_resource(resource);
}
