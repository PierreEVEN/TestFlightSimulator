#pragma once

#include <cpputils/logger.hpp>

#include <memory>
#include <ranges>
#include <unordered_map>
#include <vector>

class Node;

class IComponentType
{
  public:
    virtual void render() = 0;
};

#define MIN_REALLOC_COUNT 16

using ComponentHandle = size_t;

template <typename ComponentData_T>
using RenderFunctionType = void (*)(ComponentData_T&);

template <class ComponentData_T> class ComponentType final : public IComponentType
{
  public:

    ComponentType(RenderFunctionType<ComponentData_T> in_render_function) : render_function(in_render_function) {}

    ComponentHandle* add_component(ComponentData_T new_component)
    {
        if (component_count >= component_alloc_count)
        {
            component_alloc_count += MIN_REALLOC_COUNT;
            components = static_cast<ComponentData_T*>(realloc(components, sizeof(ComponentData_T) * component_alloc_count));
        }

        components[component_count]      = std::move(new_component);
        components[component_count].component_position = component_count;
        ComponentHandle* new_handle                    = &components[component_count].component_position;
        component_count++;
        return new_handle;
    }

    void remove_component(ComponentHandle* component_handle)
    {
        if (!component_handle)
            return;
        if (*component_handle >= component_count)
        {
            LOG_WARNING("trying to remove out of bound component : %lu (size = %lu)", *component_handle, component_count);
            return;
        }
        components[*component_handle]                    = components[component_count];
        components[*component_handle].component_position = *component_handle;
        if (component_alloc_count - component_count > MIN_REALLOC_COUNT * 2)
        {
            component_alloc_count -= MIN_REALLOC_COUNT * 2;
            components = static_cast<ComponentData_T*>(realloc(components, sizeof(ComponentData_T) * component_alloc_count));
        }
        if (component_count > 0)
            component_count--;
    }

    void render() override
    {
        for (int i = 0; i < component_count; ++i)
        {
            render_function(components[i]);
        }
    }

  private:
    RenderFunctionType<ComponentData_T> render_function;
    ComponentData_T* components            = nullptr;
    size_t           component_count       = 0;
    size_t           component_alloc_count = 0;
};

struct MyEcsData
{
    ComponentHandle component_position;
    int   val;
    float fl_test;
};

class SceneProxyECS
{
  public:
    template <typename ComponentData_T> void register_component_type(const RenderFunctionType<ComponentData_T>& render_function)
    {
        components[typeid(ComponentData_T).name()] = std::make_unique<ComponentType<ComponentData_T>>(std::move(render_function));
    }

    void render()
    {
        for (const auto& component : components | std::views::values)
            component->render();
    }

    template <typename ComponentData_T> ComponentHandle* add_component(ComponentData_T new_component)
    {
        const auto found_component = components.find(typeid(ComponentData_T).name());
        if (found_component == components.end())
        {
            LOG_ERROR("component type %s has not been registered", typeid(ComponentData_T).name());
            return nullptr;
        }
        return static_cast<ComponentType<ComponentData_T>*>(found_component->second.get())->add_component(new_component);
    }

    template <typename ComponentData_T>
    void remove_component(ComponentHandle* handle)
    {
        LOG_WARNING("handle = %lu", *handle);
        const auto found_component = components.find(typeid(ComponentData_T).name());
        if (found_component == components.end())
        {
            LOG_ERROR("component type %s has not been registered", typeid(ComponentData_T).name());
            return;
        }
        static_cast<ComponentType<ComponentData_T>*>(found_component->second.get())->remove_component(handle);
    }

  private:
    std::unordered_map<std::string, std::unique_ptr<IComponentType>> components;
};

inline void test_ecs()
{
    SceneProxyECS test;

    test.register_component_type<MyEcsData>([](MyEcsData& data) {
        data.val++;
        data.fl_test *= 2.f;
    });

    ComponentHandle* handle = test.add_component(MyEcsData{.val = 10, .fl_test = 5.f});
    LOG_WARNING("handle = %lu", *handle);
    LOG_WARNING("handle = %lu", *handle);
    test.remove_component<MyEcsData>(handle);
}

class Scene
{
  public:
  private:
    std::vector<std::shared_ptr<Node>> scene_nodes;

    SceneProxyECS scene_proxy;
};