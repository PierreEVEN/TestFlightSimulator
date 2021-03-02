#pragma once
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>


#include "assetId.h"
#include "engine/NonCopiable.h"
#include "ios/logger.h"

class Window;
class AssetBase;

class AssetManager final
{
public:

	AssetManager(Window* context) : window_context(context) {}
	~AssetManager();
	
	template <class AssetClass, typename ... Args>
	AssetClass* create(const AssetId& asset_id, Args... args) {
		if (exists(asset_id)) {
			logger_error("Cannot create two asset with the same id : %s", asset_id.to_string().c_str());
			return nullptr;
		}

		
		AssetClass* asset_ptr = static_cast<AssetClass*>(std::malloc(sizeof(AssetClass)));
		
		asset_ptr->internal_constructor(window_context, new AssetId(asset_id));
		
		new (asset_ptr) AssetClass(std::forward<Args>(args)...);
		register_lock.lock();
		assets[asset_id] = asset_ptr;
		register_lock.unlock();
		return asset_ptr;
	}

	bool exists(const AssetId& id)
	{
		return assets.find(id) != assets.end();
	}

private:

	std::mutex register_lock;
	std::unordered_map<AssetId, AssetBase*> assets;
	Window* window_context;
};


class AssetBase : public NonCopiable
{
public:
	friend class AssetManager;
	AssetBase() {}

	virtual std::string to_string() { return asset_id->to_string(); }

protected:
	virtual ~AssetBase() { delete asset_id; }

	const AssetId* asset_id;
	Window* window_context;

private:
	void internal_constructor(Window* context, const AssetId* id) {
		asset_id = id;
		window_context = context;
	}	
};