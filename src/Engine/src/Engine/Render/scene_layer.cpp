#include <Core/enum.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Graphics/scene_render_targets.hpp>

namespace Engine
{
    SceneLayer::SceneLayer()
    {}

    SceneLayer::SceneLayer(const Name& name) : m_name(name)
    {}

    bool SceneLayer::can_create_parent_layer()
    {
        return true;
    }

    SceneLayer::~SceneLayer()
    {
        if (m_next)
            delete m_next;
        m_next = nullptr;

        if (m_parent)
        {
            m_parent->m_next = nullptr;
            m_parent         = nullptr;
        }
    }

    SceneLayer& SceneLayer::clear()
    {
        return *this;
    }

    SceneLayer& SceneLayer::begin_render(SceneRenderer* renderer, RenderTargetBase* render_target)
    {
        for (auto& func : on_begin_render)
        {
            func(renderer, render_target, this);
        }

        return *this;
    }

    SceneLayer& SceneLayer::end_render(SceneRenderer* renderer, RenderTargetBase* render_target)
    {
        for (auto& func : on_end_render)
        {
            func(renderer, render_target, this);
        }

        return *this;
    }

    SceneLayer& SceneLayer::render(SceneRenderer* renderer, RenderTargetBase* render_target)
    {

        return *this;
    }

    SceneLayer* SceneLayer::parent() const
    {
        return m_parent;
    }

    SceneLayer* SceneLayer::next() const
    {
        return m_next;
    }

    const Name& SceneLayer::name() const
    {
        return m_name;
    }

    void SceneLayer::destroy()
    {
        if (m_parent)
        {
            m_parent->m_next = m_next;
        }

        if (m_next)
        {
            m_next->m_parent = m_parent;
        }

        delete this;
    }

    SceneLayer* SceneLayer::find(const Name& name)
    {
        SceneLayer* current = this;

        while (current)
        {
            if (current->m_name == name)
                return current;
            current = current->m_next;
        }

        current = this->m_parent;

        while (current)
        {
            if (current->m_name == name)
                return current;
            current = current->m_parent;
        }

        return nullptr;
    }

    SceneLayer* SceneLayer::create_next_internal(const Name& name, SceneLayer* (*allocator)(const Name& name))
    {
        SceneLayer* new_layer = find(name);

        if (new_layer)
            return new_layer;

        new_layer           = allocator(name);
        new_layer->m_parent = this;
        new_layer->m_next   = m_next;
        m_next              = new_layer;
        return new_layer;
    }

    SceneLayer* SceneLayer::create_parent_internal(const Name& name, SceneLayer* (*allocator)(const Name& name))
    {
        if (!can_create_parent_layer())
            return nullptr;

        SceneLayer* new_layer = find(name);

        if (new_layer)
            return new_layer;

        new_layer           = allocator(name);
        new_layer->m_parent = m_parent;
        new_layer->m_next   = this;
        m_parent            = new_layer;
        return new_layer;
    }

    BatchedLines* SceneLayer::batched_lines()
    {
        return nullptr;
    }

    BatchedTriangles* SceneLayer::batched_triangles()
    {
        return nullptr;
    }


    RootLayer::RootLayer() : SceneLayer("Root Layer")
    {}

    bool RootLayer::can_create_parent_layer()
    {
        return false;
    }

    BasePassSceneLayer& BasePassSceneLayer::clear()
    {
        SceneLayer::clear();
        m_components.clear();
        return *this;
    }

    BasePassSceneLayer& BasePassSceneLayer::begin_render(SceneRenderer* renderer, RenderTargetBase* render_target)
    {
        renderer->begin_rendering_target(GBuffer::instance());
        SceneLayer::begin_render(renderer, render_target);
        return *this;
    }

    BasePassSceneLayer& BasePassSceneLayer::render(SceneRenderer* renderer, RenderTargetBase* render_target)
    {
        SceneLayer::render(renderer, render_target);

        for (PrimitiveComponent* component : m_components)
        {
            component->render(renderer, render_target, this);
        }
        return *this;
    }

    BasePassSceneLayer& BasePassSceneLayer::end_render(SceneRenderer* renderer, RenderTargetBase* render_target)
    {
        SceneLayer::end_render(renderer, render_target);
        renderer->end_rendering_target();
        return *this;
    }

    SceneLayer& BasePassSceneLayer::add_component(PrimitiveComponent* component)
    {
        if (!component)
            return *this;

        m_components.insert(component);
        return *this;
    }

    SceneLayer& BasePassSceneLayer::remove_component(PrimitiveComponent* component)
    {
        m_components.erase(component);
        return *this;
    }

    const Set<PrimitiveComponent*>& BasePassSceneLayer::primitive_components() const
    {
        return m_components;
    }

    DepthRenderingLayer& DepthRenderingLayer::clear()
    {
        SceneLayer::clear();
        m_light_components.clear();
        return *this;
    }

    DepthRenderingLayer& DepthRenderingLayer::add_light(LightComponent* component)
    {
        m_light_components.insert(component);
        return *this;
    }

    DepthRenderingLayer& DepthRenderingLayer::remove_light(LightComponent* component)
    {
        m_light_components.erase(component);
        return *this;
    }

    DepthRenderingLayer& DepthRenderingLayer::render(SceneRenderer* renderer, RenderTargetBase* rt)
    {
        return *this;
    }

    //// LIGHTING LAYER
    LightingSceneLayer& LightingSceneLayer::clear()
    {
        SceneLayer::clear();
        m_light_components.clear();
        return *this;
    }

    LightingSceneLayer& LightingSceneLayer::render(SceneRenderer* renderer, RenderTargetBase* rt)
    {
        SceneLayer::render(renderer, rt);
        auto& components = light_components();

        for (LightComponent* component : components)
        {
            component->render(renderer, rt, this);
        }

        return *this;
    }

    LightingSceneLayer& LightingSceneLayer::add_light(LightComponent* component)
    {
        if (!component)
            return *this;

        m_light_components.insert(component);
        return *this;
    }

    LightingSceneLayer& LightingSceneLayer::remove_light(LightComponent* component)
    {
        m_light_components.erase(component);
        return *this;
    }

    const Set<LightComponent*>& LightingSceneLayer::light_components() const
    {
        return m_light_components;
    }

    LightingSceneLayer::~LightingSceneLayer()
    {}
}// namespace Engine
