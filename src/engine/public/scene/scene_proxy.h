#pragma once
#include "assets/asset_ptr.h"
#include "rendering/window.h"

#include <glm/glm.hpp>

class ShaderBuffer;

struct ModelStruct
{
    VkBuffer  vertices;
    VkBuffer  indices;
    glm::mat4 model_matrix;
};

struct EntityHandle
{
    size_t entity_id;
    size_t type_hash;
};

template <typename Struct_T> using ProxyFunctionType = void (*)(Struct_T&, RenderContext&);

class ISceneProxyEntityGroup
{
  public:
    ISceneProxyEntityGroup(size_t type_id) : type_hash(type_id)
    {
    }
    virtual void remove_entity(const EntityHandle& in_handle) = 0;

    virtual void render_group(RenderContext& render_context) = 0;
    const size_t type_hash;
};

constexpr size_t MIN_REALLOC_SIZE = 10000;

template <typename Struct_T> class TSceneProxyEntityGroup final : public ISceneProxyEntityGroup
{
  public:
    TSceneProxyEntityGroup(const ProxyFunctionType<Struct_T>& in_function) : proxy_function(in_function), ISceneProxyEntityGroup(typeid(Struct_T).hash_code())
    {
    }

    void render_group(RenderContext& render_context) override
    {
        for (size_t i = 0; i < element_count; ++i)
        {
            proxy_function(data[element_count], render_context);
        }
    }

    EntityHandle add_entity(const Struct_T& new_element)
    {
        size_t new_element_id = element_count;

        resize(element_count + 1);
        Struct_T* new_entity_ptr = &data[new_element_id];
        memcpy(new_entity_ptr, &new_element, sizeof(Struct_T));

        size_t new_entity_handle          = create_new_entity_handle();
        handle_to_entity_map[new_entity_handle] = new_entity_ptr;
        entity_to_handle_map[new_entity_ptr]    = new_entity_handle;

        return {
            .entity_id = new_entity_handle,
            .type_hash = typeid(Struct_T).hash_code(),
        };
    }

    void remove_entity(const EntityHandle& in_handle) override
    {
        if (element_count > 0)
        {
            Struct_T* erased_entity_ptr = get_entity(in_handle);
            Struct_T* last_entity_ptr    = &data[element_count - 1];
            size_t    last_entity_handle = entity_to_handle_map[last_entity_ptr];

            if (!erased_entity_ptr)
            {
                LOG_WARNING("failed to find entity");
            }
            else
            {
                memcpy(erased_entity_ptr, last_entity_ptr, sizeof(Struct_T));
                handle_to_entity_map.erase(in_handle.entity_id);
                handle_to_entity_map[last_entity_handle] = erased_entity_ptr;
                entity_to_handle_map.erase(last_entity_ptr);
                entity_to_handle_map[erased_entity_ptr] = last_entity_handle;
                resize(element_count - 1);
            }
        }
    }

    Struct_T* get_entity(const EntityHandle& in_handle)
    {
        auto entity = handle_to_entity_map.find(in_handle.entity_id);
        if (entity != handle_to_entity_map.end())
            return entity->second;
        return nullptr;
    }

  private:
    void resize(size_t in_elem_count)
    {
        element_count = in_elem_count;

        if (allocated_size >= MIN_REALLOC_SIZE && in_elem_count < MIN_REALLOC_SIZE)
            return;

        if (in_elem_count == 0 && data)
        {
            free(data);
            data           = nullptr;
            allocated_size = 0;
            return;
        }

        if (in_elem_count > allocated_size || static_cast<int64_t>(in_elem_count) < static_cast<int64_t>(allocated_size) - static_cast<int64_t>(2 * MIN_REALLOC_SIZE))
        {
            const size_t new_allocated_size  = in_elem_count + MIN_REALLOC_SIZE;
            void*        previous_entity_ptr = data;

            size_t    new_memory_size = new_allocated_size * sizeof(Struct_T);
            Struct_T* data_storage    = static_cast<Struct_T*>(realloc(data, new_memory_size));
            if (!data_storage)
            {
                LOG_FATAL("failed to resize proxy buffer size to %lu (for %lu elements)", new_allocated_size, in_elem_count);
            }

            data           = data_storage;
            allocated_size = new_allocated_size;

            if (data != previous_entity_ptr)
            {
                void*         new_entity_ptr = data;
                const int64_t offset         = (reinterpret_cast<int64_t>(new_entity_ptr) - reinterpret_cast<int64_t>(previous_entity_ptr));
                for (auto& entity : handle_to_entity_map)
                {
                    uint64_t new_position = reinterpret_cast<uint64_t>(entity.second) + offset;
                    entity.second         = reinterpret_cast<Struct_T*>(new_position);
                }

                std::unordered_map<Struct_T*, size_t> temp_map;
                for (const auto& handle : entity_to_handle_map)
                {
                    uint64_t new_position = reinterpret_cast<uint64_t>(handle.first) + offset;
                    temp_map[reinterpret_cast<Struct_T*>(new_position)] = handle.second;
                }
                entity_to_handle_map = temp_map;
            }
        }
    }

    size_t entity_map_random_handle = 0;

    [[nodiscard]] size_t create_new_entity_handle()
    {
        size_t handle;
        do
        {
            handle = entity_map_random_handle++;
        } while (handle_to_entity_map.contains(handle));
        return handle;
    }

    Struct_T*                             data           = nullptr;
    size_t                                element_count  = 0;
    size_t                                allocated_size = 0;
    const ProxyFunctionType<Struct_T>     proxy_function;
    std::unordered_map<size_t, Struct_T*> handle_to_entity_map;
    std::unordered_map<Struct_T*, size_t> entity_to_handle_map;
};

class SceneProxy
{
  public:
    void render(RenderContext& render_context)
    {
        for (auto& group : entity_groupes)
        {
            group->render_group(render_context);
        }
    }

    template <typename Struct_T> TSceneProxyEntityGroup<Struct_T>* find_entity_group()
    {
        const size_t type_hash = typeid(Struct_T).hash_code();
        for (const auto& group : entity_groupes)
        {
            if (group->type_hash == type_hash)
            {
                return static_cast<TSceneProxyEntityGroup<Struct_T>*>(group.get());
            }
        }
        return nullptr;
    }

    template <typename Struct_T> void register_entity_type(ProxyFunctionType<Struct_T> render_function)
    {
        if (find_entity_group<Struct_T>())
        {
            LOG_ERROR("already registered entity type %s", typeid(Struct_T).name());
            return;
        }
        entity_groupes.emplace_back(std::make_shared<TSceneProxyEntityGroup<Struct_T>>(render_function));
    }

    template <typename Struct_T> EntityHandle add_entity(const Struct_T& in_struct)
    {
        if (auto* group = find_entity_group<Struct_T>())
        {
            return group->add_entity(in_struct);
        }
        return {
            .entity_id = 0,
            .type_hash = typeid(Struct_T).hash_code(),
        };
    }

    void remove_entity(const EntityHandle& entity_handle)
    {
        for (auto& group : entity_groupes)
        {
            if (entity_handle.type_hash == group->type_hash)
            {
                group->remove_entity(entity_handle);
                return;
            }
        }
    }

  private:
    ModelStruct*            model_data        = nullptr;
    TAssetPtr<ShaderBuffer> global_model_ssbo = nullptr;

    std::vector<std::shared_ptr<ISceneProxyEntityGroup>> entity_groupes;
};