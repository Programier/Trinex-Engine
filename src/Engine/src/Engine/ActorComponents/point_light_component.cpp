#include <Core/class.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine.hpp>
#include <Core/property.hpp>
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
    {
        Class* self = static_class_instance();
        self->add_properties(new FloatProperty("Radius", "Radius of this light", &This::radius));
    }

    PointLightComponent::PointLightComponent() : radius(10.f)
    {}

    LightComponent::Type PointLightComponent::light_type() const
    {
        return LightComponent::Type::Point;
    }

    PointLightComponent& PointLightComponent::add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer)
    {
        scene->deferred_lighting_layer()->add_light(this);
        return *this;
    }

#define get_param(param_name, type) reinterpret_cast<type*>(material->find_parameter(name_##param_name));
    PointLightComponent& PointLightComponent::render(class SceneRenderer* renderer, class RenderTargetBase*, class SceneLayer*)
    {
        Material* material                 = DefaultResources::point_light_material;
        static Name name_light_color       = "light_color";
        static Name name_light_radius      = "light_radius";
        static Name name_light_intensivity = "light_intensivity";
        static Name name_light_location    = "light_location";
        static Name name_ambient_color     = "ambient_color";

        Vec3MaterialParameter* color_parameter         = get_param(light_color, Vec3MaterialParameter);
        Vec3MaterialParameter* location_parameter      = get_param(light_location, Vec3MaterialParameter);
        Vec3MaterialParameter* ambient_color_parameter = get_param(ambient_color, Vec3MaterialParameter);
        FloatMaterialParameter* radius_parameter       = get_param(light_radius, FloatMaterialParameter);
        FloatMaterialParameter* intensivity_parameter  = get_param(light_intensivity, FloatMaterialParameter);

        if (color_parameter)
        {
            color_parameter->param = light_color;
        }

        if (location_parameter)
        {
            location_parameter->param = world_transform().location();
        }

        if (radius_parameter)
        {
            radius_parameter->param = radius;
        }

        if (intensivity_parameter)
        {
            intensivity_parameter->param = intensivity;
        }

        if (ambient_color_parameter)
        {
            ambient_color_parameter->param = renderer->ambient_light();
        }

        material->apply();
        DefaultResources::screen_position_buffer->rhi_bind(0, 0);
        engine_instance->rhi()->draw(6);
        return *this;
    }
}// namespace Engine
