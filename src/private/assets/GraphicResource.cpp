

#include "assets/GraphicResource.h"

#include "rendering/window.h"

AssetRef::AssetRef(const std::string& name) : AssetRef(std::hash<std::string>{}(name))
{
#if _DEBUG
		asset_name = name;
#endif
}

std::string AssetRef::to_string() const
{
#if _DEBUG
	return asset_name;
#else
	return std::to_string(id);
#endif
}

GraphicResource::GraphicResource(Window* context, const AssetRef& asset_reference)
	: asset_ref(asset_reference)
{
	context->get_resource_manager()->register_resource(this);
}
