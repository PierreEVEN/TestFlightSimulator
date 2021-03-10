#pragma once
#include <memory>

#include "assetId.h"

class Window;
class AssetBase;

class IAssetPtr
{
public:
	IAssetPtr();
	IAssetPtr(Window* context, const AssetId& in_asset_id);
	IAssetPtr(AssetBase* in_asset);
	explicit IAssetPtr(const IAssetPtr& other) : window_context(other.window_context), asset_id(other.asset_id), asset(other.asset) {}

	void set(AssetBase* in_asset);
	void set(Window* context, const AssetId& in_asset_id);
	void clear();

	[[nodiscard]] AssetBase* get();
	[[nodiscard]] AssetId id() const;

	virtual ~IAssetPtr();

private:
	Window* window_context = nullptr;
	std::shared_ptr<AssetId> asset_id;
	AssetBase* asset = nullptr;	
};

template<class AssetClass>
class TAssetPtr final : public IAssetPtr
{
public:
	TAssetPtr() : IAssetPtr() {}
	TAssetPtr(Window* context, const AssetId& in_asset_id) : IAssetPtr(context, in_asset_id) {}
	TAssetPtr(AssetClass* in_asset) : IAssetPtr(static_cast<AssetBase*>(in_asset)) {}

	AssetClass* operator->() { return static_cast<AssetClass*>(get()); }
};
