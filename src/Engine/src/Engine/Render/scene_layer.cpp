#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/Render/scene_renderer.hpp>

namespace Engine
{
    const Name SceneLayer::name_clear_render_targets = "Clear Render Targets";
    const Name SceneLayer::name_base_pass            = "Base Pass";
    const Name SceneLayer::name_light_pass           = "Light Pass";
    const Name SceneLayer::name_scene_output_pass    = "Scene Output Pass";
    const Name SceneLayer::name_post_process         = "Post Process";

    SceneLayer::SceneLayer(const Name& name) : m_name(name)
    {}

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
        for (PrimitiveComponent* component : m_components)
        {
            component->m_layer = nullptr;
        }
        m_components.clear();
        m_light_components.clear();
        lines.clear();
        return *this;
    }

    SceneLayer& SceneLayer::render(SceneRenderer* renderer, RenderTargetBase* render_target)
    {
        for (auto& method : begin_render_methods_callbacks)
        {
            (renderer->*method)(render_target, this);
        }

        for (auto& func : begin_render_function_callbacks)
        {
            func(renderer, render_target, this);
        }

        for (PrimitiveComponent* component : m_components)
        {
            component->render(renderer, render_target, this);
        }

        for(LightComponent* component : m_light_components)
        {
            component->render(renderer, render_target, this);
        }

        lines.render(renderer->scene_view());

        for (auto& func : end_render_function_callbacks)
        {
            func(renderer, render_target, this);
        }

        for (auto& method : end_render_methods_callbacks)
        {
            (renderer->*method)(render_target, this);
        }
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

    SceneLayer* SceneLayer::create_next(const Name& name)
    {
        SceneLayer* new_layer = find(name);

        if (new_layer)
            return new_layer;

        new_layer           = new SceneLayer(name);
        new_layer->m_parent = this;
        new_layer->m_next   = m_next;
        m_next              = new_layer;
        return new_layer;
    }

    SceneLayer* SceneLayer::create_parent(const Name& name)
    {
        if (!m_can_create_parent)
            return nullptr;

        SceneLayer* new_layer = find(name);

        if (new_layer)
            return new_layer;

        new_layer           = new SceneLayer(name);
        new_layer->m_parent = m_parent;
        new_layer->m_next   = this;
        m_parent            = new_layer;
        return new_layer;
    }

    SceneLayer& SceneLayer::add_component(PrimitiveComponent* component)
    {
        if (!component || component->m_layer == this)
            return *this;

        if (component->m_layer != nullptr)
        {
            component->m_layer->remove_component(component);
        }

        m_components.insert(component);
        return *this;
    }

    SceneLayer& SceneLayer::remove_component(PrimitiveComponent* component)
    {
        if (!component || component->m_layer != this)
            return *this;

        m_components.erase(component);
        component->m_layer = nullptr;
        return *this;
    }

    SceneLayer& SceneLayer::add_light(LightComponent* component)
    {
        if (!component || component->m_layer == this)
            return *this;

        if (component->m_layer != nullptr)
        {
            component->m_layer->remove_light(component);
        }

        m_light_components.insert(component);
        return *this;
    }

    SceneLayer& SceneLayer::remove_light(LightComponent* component)
    {
        if (!component || component->m_layer != this)
            return *this;

        m_light_components.erase(component);
        component->m_layer = nullptr;
        return *this;
    }
}// namespace Engine
