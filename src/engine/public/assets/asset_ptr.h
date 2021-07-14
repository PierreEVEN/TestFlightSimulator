#pragma once

#include <memory>

#include "asset_id.h"

class Window;
class AssetBase;

class IAssetPtr
{
  public:
    IAssetPtr();
    IAssetPtr(const AssetId& in_asset_id);
    IAssetPtr(AssetBase* in_asset);
    explicit IAssetPtr(const IAssetPtr& other) : asset_id(other.asset_id), asset(other.asset)
    {
    }

    void set(AssetBase* in_asset);
    void set(const AssetId& in_asset_id);
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
        return asset && asset_id;
    }

  private:
    std::shared_ptr<AssetId> asset_id = nullptr;
    AssetBase*               asset    = nullptr;
};

template <class AssetClass> class TAssetPtr final : public IAssetPtr
{
  public:
    TAssetPtr() : IAssetPtr()
    {
    }
    TAssetPtr(const AssetId& in_asset_id) : IAssetPtr(in_asset_id)
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
