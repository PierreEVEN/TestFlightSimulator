#pragma once
#include "assetId.h"

class Window;
class AssetBase;

class IAssetPtr
{
public:
	IAssetPtr();
	IAssetPtr(Window* context, const AssetId& in_asset_id);
	IAssetPtr(AssetBase* in_asset);

	void set(AssetBase* in_asset);
	void set(Window* context, const AssetId& in_asset_id);
	void clear();

	[[nodiscard]] AssetBase* get();
	[[nodiscard]] AssetId id() const;

	virtual ~IAssetPtr();

private:
	Window* window_context = nullptr;
	AssetId* asset_id = nullptr;
	AssetBase* asset = nullptr;	
};

template<class AssetClass>
class TAssetPtr final : public IAssetPtr
{
public:
	TAssetPtr() : IAssetPtr() {}
	TAssetPtr(Window* context, const AssetId& in_asset_id) : IAssetPtr(context, in_asset_id) {}
	TAssetPtr(AssetClass* in_asset) : IAssetPtr(in_asset) {}

	AssetClass* operator->() { return static_cast<AssetClass*>(get()); }
};
