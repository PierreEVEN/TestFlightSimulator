
#include "assets/assetPtr.h"

#include "IEngineInterface.h"
#include "assets/assetBase.h"
#include "rendering/window.h"

IAssetPtr::IAssetPtr() { clear(); }

IAssetPtr::IAssetPtr(IEngineInterface* in_engine_interface, const AssetId& in_asset_id) { set(in_engine_interface, in_asset_id); }

IAssetPtr::IAssetPtr(AssetBase* in_asset) { set(in_asset); }

void IAssetPtr::set(AssetBase* in_asset)
{
    if (in_asset == asset) return;
    clear();
    if (!in_asset) return;
    asset_id         = std::make_shared<AssetId>(in_asset->get_id());
    asset            = in_asset;
    engine_interface = asset->get_engine_interface();
}

void IAssetPtr::set(IEngineInterface* in_engine_interface, const AssetId& in_asset_id)
{
    if (asset_id && in_asset_id == *asset_id) return;
    clear();
    asset_id         = std::make_shared<AssetId>(in_asset_id);
    engine_interface = in_engine_interface;
    asset            = in_engine_interface->get_asset_manager()->find(*asset_id);
}

void IAssetPtr::clear()
{
    engine_interface = nullptr;
    asset            = nullptr;
    asset_id         = nullptr;
}

AssetBase* IAssetPtr::get()
{
    if (!engine_interface) return nullptr;
    if (!asset_id) return nullptr;
    if (!asset) asset = engine_interface->get_asset_manager()->find(*asset_id);
    if (asset)
    {
        if (!asset->try_load()) return nullptr;
        return asset;
    }
    return nullptr;
}

IAssetPtr::~IAssetPtr() {}
