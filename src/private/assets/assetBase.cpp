

#include "assets/assetBase.h"

AssetManager::~AssetManager()
{
	std::lock_guard<std::mutex> lock(register_lock);
	for (auto& item : assets) delete item.second;
}
