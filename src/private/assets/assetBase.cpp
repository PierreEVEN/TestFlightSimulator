

#include "assets/assetBase.h"

AssetManager::~AssetManager()
{
	std::lock_guard<std::mutex> lock(register_lock);
	for (auto& item : assets) delete item.second;
}

AssetBase* AssetManager::find(const AssetId& id) const
{
	auto asset = assets.find(id);
	if (asset == assets.end()) return nullptr;
	return asset->second;
}
