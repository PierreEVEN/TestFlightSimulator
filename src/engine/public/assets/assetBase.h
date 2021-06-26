#pragma once
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

#include "assetId.h"
#include "assetPtr.h"
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

        asset_ptr->internal_constructor(engine_interface, new AssetId(asset_id));

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
    AssetBase() {}

    virtual std::string to_string() { return asset_id->to_string(); }

    [[nodiscard]] const AssetId& get_id() const { return *asset_id; }

    virtual bool try_load() = 0;

    IEngineInterface* get_engine_interface() const { return engine_interface; }

  protected:
    virtual ~AssetBase() { delete asset_id; }

    const AssetId* asset_id;

  private:
    void internal_constructor(IEngineInterface* in_engine_interface, const AssetId* id)
    {
        engine_interface = in_engine_interface;
        asset_id    = id;
    }
    IEngineInterface* engine_interface;
};