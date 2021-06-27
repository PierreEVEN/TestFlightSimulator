#pragma once

#include <cpputils/logger.hpp>

#include <memory>
#include <vector>

class Node;

class IComponentType
{
  public:
    virtual void render() = 0;
};

#define MIN_REALLOC_COUNT 16

using ComponentHandle = size_t;

struct ComponentBase
{
    ComponentHandle component_position;
};

template <class ComponentData_T> class ComponentType final : public IComponentType
{
  public:
    using RenderFunctionType = void (*)(ComponentData_T&);

    RenderFunctionType render_function;

    ComponentHandle* add_components(ComponentData_T new_component)
    {
        if (component_count >= component_alloc_count)
        {
            component_alloc_count += MIN_REALLOC_COUNT;
            components = static_cast<ComponentData_T*>(realloc(components, sizeof(ComponentData_T) * component_alloc_count));
        }

        new_component.component_position = component_count;
        ComponentHandle* new_handle      = &new_component.component_position;
        components[component_count++]    = std::move(new_component);

        return new_handle;
    }

    void remove_component(ComponentHandle component_handle)
    {
        components[component_handle]                    = components[component_count - 1];
        components[component_handle].component_position = component_handle;
    }

    void render() override
    {
        for (int i = 0; i < component_count; ++i)
        {
            render_function(components[i]);
        }
    }

  private:
    ComponentData_T* components            = nullptr;
    size_t           component_count       = 0;
    size_t           component_alloc_count = 0;
};

class SceneProxyECS
{
  public:
    template <typename T> void register_component_type(const ComponentType<T>::RenderFunctionType& render_function)
    {
        components.emplace_back(std::make_unique<ComponentType<T>>(std::move(render_function)));
    }

    void render()
    {
        for (const auto& item : components)
            item->render();
    }

  private:
    std::vector<std::unique_ptr<IComponentType>> components;
};

struct MyEcsData
{
    int   val;
    float fl_test;
};

inline void test()
{
    SceneProxyECS test;

    test.register_component_type<MyEcsData>([](MyEcsData& data) {
        data.val++;
        data.fl_test *= 2.f;
    });
}

class Scene
{
  public:
  private:
    std::vector<std::shared_ptr<Node>> scene_nodes;

    SceneProxyECS scene_proxy;
};
