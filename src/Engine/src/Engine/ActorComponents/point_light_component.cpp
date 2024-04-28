#include <Core/class.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine.hpp>
#include <Core/property.hpp>
#include <Core/render_thread.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_engine_class(PointLightComponent, 0);
    implement_initialize_class(PointLightComponent)
    {}

    PointLightComponent::PointLightComponent()
    {}

    LightComponent::Type PointLightComponent::light_type() const
    {
        return LightComponent::Type::Point;
    }


    PointLightComponent& PointLightComponent::submit_point_light_data()
    {
        return *this;
    }

    SceneRenderer& SceneRenderer::add_component(PointLightComponent* component, Scene* scene)
    {
        add_base_component(component, scene);

        if (component->leaf_class_is<PointLightComponent>())
        {
            deferred_lighting_layer()->add_light(component);
        }
        return *this;
    }

    PointLightComponent& PointLightComponent::start_play()
    {
        Super::start_play();
        submit_point_light_data();
        return *this;
    }

    PointLightComponent& PointLightComponent::add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer)
    {
        renderer->add_component(this, scene);
        return *this;
    }

#define get_param(param_name, type) reinterpret_cast<type*>(material->find_parameter(Name::param_name));
    SceneRenderer& SceneRenderer::render_component(PointLightComponent* component, RenderTargetBase* rt, SceneLayer* layer)
    {
        render_base_component(component, rt, layer);

        if (!component->is_enabled || !component->leaf_class_is<PointLightComponent>())
            return *this;

        Material* material = DefaultResources::point_light_material;

        Vec3MaterialParameter* color_parameter        = get_param(color, Vec3MaterialParameter);
        Vec3MaterialParameter* location_parameter     = get_param(location, Vec3MaterialParameter);
        FloatMaterialParameter* intensivity_parameter = get_param(intensivity, FloatMaterialParameter);
        FloatMaterialParameter* radius_parameter      = get_param(radius, FloatMaterialParameter);

        if (color_parameter)
        {
            color_parameter->param = component->light_color;
        }

        if (location_parameter)
        {
            location_parameter->param = component->proxy()->world_transform().location();
        }

        if (intensivity_parameter)
        {
            intensivity_parameter->param = component->intensivity;
        }

        if (radius_parameter)
        {
            radius_parameter->param = component->proxy()->attenuation_radius();
        }

        material->apply();
        DefaultResources::screen_position_buffer->rhi_bind(0, 0);
        engine_instance->rhi()->draw(6);
        return *this;
    }

    PointLightComponent& PointLightComponent::render(class SceneRenderer* renderer, class RenderTargetBase* rt,
                                                     class SceneLayer* layer)
    {
        renderer->render_component(this, rt, layer);
        return *this;
    }

    ActorComponentProxy* PointLightComponent::create_proxy()
    {
        return new PointLightComponentProxy();
    }

    PointLightComponentProxy* PointLightComponent::proxy() const
    {
        return typed_proxy<PointLightComponentProxy>();
    }
}// namespace Engine
