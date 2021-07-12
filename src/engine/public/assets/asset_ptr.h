#pragma once

#include <memory>

#include "asset_id.h"

class AssetManager;
class Window;
class AssetBase;

class IAssetPtr
{
  public:
    IAssetPtr();
    IAssetPtr(AssetManager* in_asset_manager, const AssetId& in_asset_id);
    IAssetPtr(AssetBase* in_asset);
    explicit IAssetPtr(const IAssetPtr& other) : asset_manager(other.asset_manager), asset_id(other.asset_id), asset(other.asset)
    {
    }

    void set(AssetBase* in_asset);
    void set(AssetManager* asset_manager, const AssetId& in_asset_id);
    void clear();

    [[nodiscard]] AssetBase* get();
    [[nodiscard]] AssetBase* get_const() const;
    [[nodiscard]] AssetId    id() const
    {
        return *asset_id.get();
    }

    virtual ~IAssetPtr();

    bool operator!() const
    {
        return !asset;
    }

    explicit operator bool() const
    {
        return asset && asset_manager && asset_id;
    }

  private:
    AssetManager*            asset_manager = nullptr;
    std::shared_ptr<AssetId> asset_id      = nullptr;
    AssetBase*               asset         = nullptr;
};

template <class AssetClass> class TAssetPtr final : public IAssetPtr
{
  public:
    TAssetPtr() : IAssetPtr()
    {
    }
    TAssetPtr(AssetManager* in_asset_manager, const AssetId& in_asset_id) : IAssetPtr(in_asset_manager, in_asset_id)
    {
    }
    TAssetPtr(AssetClass* in_asset) : IAssetPtr(static_cast<AssetBase*>(in_asset))
    {
    }

    AssetClass* operator->()
    {
        return static_cast<AssetClass*>(get());
    }

    AssetClass* operator->() const
    {
        return static_cast<AssetClass*>(get_const());
    }
};
