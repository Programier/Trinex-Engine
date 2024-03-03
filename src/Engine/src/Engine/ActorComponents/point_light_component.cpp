#include <Core/class.hpp>
#include <Core/property.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>

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
}// namespace Engine
