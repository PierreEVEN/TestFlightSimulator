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
class IEngineInterface;


class AssetManager final
{
  public:
    AssetManager(IEngineInterface* in_engine_interface) : engine_interface(in_engine_interface) {}
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
        asset_ptr->internal_constructor(engine_interface, asset_id);

        new (asset_ptr) AssetClass(std::forward<Args>(args)...);
        register_lock.lock();
        assets[asset_id] = asset_ptr;
        register_lock.unlock();
        return asset_ptr;
    }

    bool exists(const AssetId& id) { return assets.find(id) != assets.end(); }

    [[nodiscard]] AssetBase* find(const AssetId& id);

    std::unordered_map<AssetId, AssetBase*> get_assets();

  private:
    std::mutex                              register_lock;
    std::unordered_map<AssetId, AssetBase*> assets;
    IEngineInterface*                             engine_interface;
};

class AssetBase : public NonCopiable
{
  public:
    friend class AssetManager;

    virtual std::string to_string() { return asset_id->to_string(); }

    [[nodiscard]] AssetId get_id() const
    {
        if (!asset_id)
            LOG_FATAL("asset id is null : %x", this);
        return AssetId (* asset_id);
    }

    virtual bool try_load()
    {
        return false;
    }

    [[nodiscard]] IEngineInterface* get_engine_interface() const { return engine_interface; }

    virtual ~AssetBase()
    {
        delete asset_id;
        asset_id = nullptr;
    }

  protected:
    AssetBase() = default;


  private:
    void internal_constructor(IEngineInterface* in_engine_interface, const AssetId& id)
    {
        engine_interface = in_engine_interface;
        asset_id         = new AssetId(id);
    }

    AssetId* asset_id;
    IEngineInterface* engine_interface;
};