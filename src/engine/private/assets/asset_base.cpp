

#include "assets/asset_base.h"

AssetManager::~AssetManager()
{
    std::lock_guard<std::mutex> lock(register_lock);
    for (auto& item : assets) delete item.second;
}

AssetBase* AssetManager::find(const AssetId& id)
{
    std::lock_guard<std::mutex> lock(register_lock);
    auto                        asset = assets.find(id);
    if (asset == assets.end()) return nullptr;
    return asset->second;
}

std::unordered_map<AssetId, AssetBase*> AssetManager::get_assets()
{
    std::lock_guard<std::mutex> lock(register_lock);
    return assets;
}

AssetId AssetManager::find_valid_asset_id(const std::string& asset_name)
{
    if (!find(asset_name))
        return asset_name;

    int asset_index = 1;
    while (find(asset_name + "_" + std::to_string(asset_index)))
    {
        asset_index++;
    }    

    return asset_name + "_" + std::to_string(asset_index);
}
