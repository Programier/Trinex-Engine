#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/property.hpp>
#include <Core/render_thread.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    static const AABB_3Df light_bounds({-1.f , -1.f, -1.f}, {1.f, 1.f, 1.f});

    LightComponentProxy& LightComponentProxy::bounding_box(const AABB_3Df& bounds)
    {
        m_bounds = bounds;
        return *this;
    }

    const AABB_3Df& LightComponentProxy::bounding_box() const
    {
        return m_bounds;
    }


    implement_engine_class(LightComponent, 0);
    implement_initialize_class(LightComponent)
    {
        Class* self = static_class_instance();
        self->add_properties(
                new BoolProperty("Is Enabled", "Is light enabled", &This::is_enabled),
                new BoolProperty("Enable Shadows", "The light source can cast real-time shadows", &This::enable_shadows),
                new Color3Property("Color", "Color of this light", &This::light_color),
                new FloatProperty("Intensivity", "Intensivity of this light", &This::intensivity));
    }

    LightComponent::LightComponent() : light_color({1.0, 1.0, 1.0}), intensivity(30.f), is_enabled(true), enable_shadows(false)
    {}

    LightComponent& LightComponent::on_transform_changed()
    {
        Super::on_transform_changed();

        if (Scene* world_scene = scene())
        {
            world_scene->update_light_transform(this);
        }

        return *this;
    }

    LightComponent& LightComponent::start_play()
    {
        Super::start_play();
        Scene* world_scene = scene();
        if (world_scene)
        {
            world_scene->add_light(this);
        }
        return *this;
    }

    SceneRenderer& SceneRenderer::add_component(LightComponent* component, Scene* scene)
    {
        return *this;
    }

    LightComponent& LightComponent::add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer)
    {
        renderer->add_component(this, scene);
        return *this;
    }

    SceneRenderer& SceneRenderer::render_component(LightComponent* component, RenderTargetBase* rt, SceneLayer* layer)
    {
        return *this;
    }

    LightComponent& LightComponent::render(SceneRenderer* renderer, class RenderTargetBase* rt, class SceneLayer* layer)
    {
        renderer->render_component(this, rt, layer);
        return *this;
    }

    ActorComponentProxy* LightComponent::create_proxy()
    {
        return new LightComponentProxy();
    }

    LightComponentProxy* LightComponent::proxy() const
    {
        return typed_proxy<LightComponentProxy>();
    }

    LightComponent& LightComponent::stop_play()
    {
        Super::stop_play();

        Scene* world_scene = scene();

        if (world_scene)
        {
            world_scene->remove_light(this);
        }
        return *this;
    }

    const AABB_3Df& LightComponent::bounding_box() const
    {
        return m_bounds;
    }

    void LightComponent::submit_bounds_to_render_thread()
    {
        if (LightComponentProxy* component_proxy = proxy())
        {
            render_thread()->insert_new_task<UpdateVariableCommand<AABB_3Df>>(m_bounds, component_proxy->m_bounds);
        }
    }

    LightComponent& LightComponent::update_bounding_box()
    {
        m_bounds = AABB_3Df(light_bounds).center(world_transform().location());
        submit_bounds_to_render_thread();
        return *this;
    }

    SceneLayer* LightComponent::scene_layer() const
    {
        return m_layer;
    }

    LightComponent::~LightComponent()
    {}
}// namespace Engine
