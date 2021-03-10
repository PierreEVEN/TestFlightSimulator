

#include "assets/assetBase.h"

AssetManager::~AssetManager()
{
	std::lock_guard<std::mutex> lock(register_lock);
	for (auto& item : assets) delete item.second;
}

AssetBase* AssetManager::find(const AssetId& id)
{
	std::lock_guard<std::mutex> lock(register_lock);
	auto asset = assets.find(id);
	if (asset == assets.end()) return nullptr;
	return asset->second;
}

std::unordered_map<AssetId, AssetBase*> AssetManager::get_assets() {
	std::lock_guard<std::mutex> lock(register_lock);
	return assets;	
}
