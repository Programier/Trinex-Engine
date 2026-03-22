#include <Core/math/math.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>
#include <Engine/Render/lighting.hpp>

namespace Trinex
{
	trinex_implement_engine_class(PointLightComponent, 0)
	{
		trinex_refl_prop(m_source_radius)->tooltip("Source radius of this light");
		trinex_refl_prop(m_fall_off_exponent)->tooltip("Fall Off Exponent of this light");
	}

	PointLightComponent::PointLightComponent() : m_fall_off_exponent(2.f) {}

	PointLightComponent& PointLightComponent::render_parameters(LightRenderParameters& out)
	{
		Super::render_parameters(out);
		out.source_radius     = m_source_radius;
		out.fall_off_exponent = m_fall_off_exponent;
		return *this;
	}

	float PointLightComponent::calculate_light_intensity() const
	{
		if (intensity_units() == LightUnits::Lumens)
		{
			return intensity() / (4.f * Math::pi());
		}

		return Super::calculate_light_intensity();
	}
}// namespace Trinex
