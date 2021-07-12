
#include "assets/asset_ptr.h"

#include "assets/asset_base.h"
#include "engine_interface.h"
#include "rendering/window.h"

IAssetPtr::IAssetPtr()
{
    clear();
}

IAssetPtr::IAssetPtr(AssetManager* in_asset_manager, const AssetId& in_asset_id)
{
    set(in_asset_manager, in_asset_id);
}

IAssetPtr::IAssetPtr(AssetBase* in_asset)
{
    set(in_asset);
}

void IAssetPtr::set(AssetBase* in_asset)
{
    if (in_asset == asset)
        return;
    clear();
    if (!in_asset)
        return;
    asset_id      = std::make_shared<AssetId>(in_asset->get_id());
    asset         = in_asset;
    asset_manager = asset->get_engine_interface()->get_asset_manager();
}

void IAssetPtr::set(AssetManager* in_asset_manager, const AssetId& in_asset_id)
{
    if (asset_id && in_asset_id == *asset_id)
        return;
    clear();
    asset_id      = std::make_shared<AssetId>(in_asset_id);
    asset_manager = in_asset_manager;
    asset         = asset_manager->find(*asset_id);
}

void IAssetPtr::clear()
{
    asset_manager = nullptr;
    asset         = nullptr;
    asset_id      = nullptr;
}

AssetBase* IAssetPtr::get()
{
    if (!asset_manager)
        return nullptr;
    if (!asset_id)
        return nullptr;
    if (!asset)
        asset = asset_manager->find(*asset_id);
    if (asset)
    {
        return asset;
    }
    return nullptr;
}

AssetBase* IAssetPtr::get_const() const
{
    if (!asset_manager)
        return nullptr;
    if (!asset_id)
        return nullptr;
    if (asset)
    {
        return asset;
    }
    return nullptr;
}

IAssetPtr::~IAssetPtr()
{
}
