#include <Core/class.hpp>
#include <Core/property.hpp>
#include <Core/render_thread.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/local_light_component.hpp>
#include <Engine/Render/scene_renderer.hpp>

namespace Engine
{
    float LocalLightComponentProxy::attenuation_radius() const
    {
        return m_attenuation_radius;
    }

    LocalLightComponentProxy& LocalLightComponentProxy::attenuation_radius(float value)
    {
        m_attenuation_radius = value;
        return *this;
    }

    implement_engine_class(LocalLightComponent, 0);
    implement_initialize_class(LocalLightComponent)
    {
        Class* self = static_class_instance();

        static auto on_prop_changed = [](void* object) {
            LocalLightComponent* component = reinterpret_cast<LocalLightComponent*>(object);
            component->submit_local_light_info();
        };

        auto attenuation_property =
                new FloatProperty("Attenuation radius", "Attenuation radius of this light", &This::m_attenuation_radius);
        attenuation_property->on_prop_changed.push(on_prop_changed);

        self->add_property(attenuation_property);
    }

    LocalLightComponent::LocalLightComponent() : m_attenuation_radius(30.f)
    {}

    LocalLightComponent& LocalLightComponent::submit_local_light_info()
    {
        render_thread()->insert_new_task<UpdateVariableCommand<float>>(m_attenuation_radius, proxy()->m_attenuation_radius);
        return *this;
    }

    float LocalLightComponent::attenuation_radius() const
    {
        return m_attenuation_radius;
    }

    LocalLightComponent& LocalLightComponent::attenuation_radius(float value)
    {
        m_attenuation_radius = value;
        return submit_local_light_info();
    }

    LocalLightComponent& LocalLightComponent::start_play()
    {
        Super::start_play();
        submit_local_light_info();
        return *this;
    }

    ActorComponentProxy* LocalLightComponent::create_proxy()
    {
        return new LocalLightComponentProxy();
    }

    LocalLightComponentProxy* LocalLightComponent::proxy() const
    {
        return typed_proxy<LocalLightComponentProxy>();
    }

    SceneRenderer& SceneRenderer::add_component(LocalLightComponent* component, Scene* scene)
    {
        add_base_component(component, scene);
        return *this;
    }

    LocalLightComponent& LocalLightComponent::add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer)
    {
        renderer->add_component(this, scene);
        return *this;
    }

    SceneRenderer& SceneRenderer::render_component(LocalLightComponent* component, RenderTargetBase* rt, SceneLayer* layer)
    {
        render_base_component(component, rt, layer);
        return *this;
    }

    LocalLightComponent& LocalLightComponent::render(class SceneRenderer* renderer, class RenderTargetBase* rt,
                                                     class SceneLayer* layer)
    {
        renderer->render_component(this, rt, layer);
        return *this;
    }
}// namespace Engine
