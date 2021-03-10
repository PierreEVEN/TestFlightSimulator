
#include "assets/assetPtr.h"

#include "assets/assetBase.h"
#include "rendering/window.h"

IAssetPtr::IAssetPtr()
{
	clear();
}

IAssetPtr::IAssetPtr(Window* context, const AssetId& in_asset_id)
{
	set(context, in_asset_id);
}

IAssetPtr::IAssetPtr(AssetBase* in_asset)
{
	set(in_asset);	
}

void IAssetPtr::set(AssetBase* in_asset)
{
	if (in_asset == asset) return;
	clear();
	if (!in_asset) return;
	asset_id = std::make_shared<AssetId>(in_asset->get_id());
	asset = in_asset;
	window_context = asset->get_context();	
}

void IAssetPtr::set(Window* context, const AssetId& in_asset_id)
{
	if (in_asset_id == *asset_id) return;
	clear();
	asset_id = std::make_shared<AssetId>(in_asset_id);
	window_context = context;
	asset = window_context->get_asset_manager()->find(*asset_id);
}

void IAssetPtr::clear()
{
	window_context = nullptr;
	asset = nullptr;
	asset_id = nullptr;
}

AssetBase* IAssetPtr::get()
{
	if (!window_context) return nullptr;
	if (!asset_id) return nullptr;
	if (!asset) asset = window_context->get_asset_manager()->find(*asset_id);
	if (asset)
	{
		if (!asset->try_load()) return nullptr;
		return asset;
	}
	return nullptr;
}

IAssetPtr::~IAssetPtr() {}
