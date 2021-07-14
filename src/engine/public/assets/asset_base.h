#pragma once
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

#include "asset_id.h"
#include "asset_ptr.h"
#include "types/nonCopiable.h"

#include <cpputils/logger.hpp>

class GfxContext;
class Window;
class AssetBase;

class AssetManager
{

  public:
    AssetManager() = default;

    template <typename AssetManager_T, typename... Args> static void initialize(Args&&... arguments)
    {
        set(std::make_shared<AssetManager_T>(std::forward<Args>(arguments)...));
    }

    template <typename AssetManager_T = AssetManager> static AssetManager_T* get()
    {
        return static_cast<AssetManager_T*>(get_internal().get());
    }

    static void destroy();

    ~AssetManager();

    template <class AssetClass, typename... Args> TAssetPtr<AssetClass> create(const AssetId& asset_id, Args... args)
    {
        if (exists(asset_id))
        {
            LOG_ERROR("Cannot create two asset with the same id : %s", asset_id.to_string().c_str());
            return nullptr;
        }

        AssetClass* asset_ptr = static_cast<AssetClass*>(std::malloc(sizeof(AssetClass)));
        if (!asset_ptr)
            LOG_FATAL("failed to create asset storage");
        asset_ptr->internal_constructor(asset_id);

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

    [[nodiscard]] AssetBase*                              find(const AssetId& id);
    [[nodiscard]] AssetId                                 find_valid_asset_id(const std::string& asset_name);
    [[nodiscard]] std::unordered_map<AssetId, AssetBase*> get_assets();

  private:
    static void                          set(std::shared_ptr<AssetManager> in_asset_manager);
    static std::shared_ptr<AssetManager> get_internal();

    std::mutex                              register_lock;
    std::unordered_map<AssetId, AssetBase*> assets;
};

class AssetBase : public NonCopiable
{
  public:
    friend class AssetManager;

    virtual std::string to_string()
    {
        return asset_id->to_string();
    }

    [[nodiscard]] AssetId get_id() const
    {
        if (!asset_id)
            LOG_FATAL("asset id is null : %x", this);
        return AssetId(*asset_id);
    }

    virtual bool try_load()
    {
        return false;
    }

    virtual ~AssetBase()
    {
        delete asset_id;
        asset_id = nullptr;
    }

  protected:
    AssetBase()
    {
    }

  private:
    void internal_constructor(const AssetId& id)
    {
        asset_id = new AssetId(id);
    }

    AssetId* asset_id;
};