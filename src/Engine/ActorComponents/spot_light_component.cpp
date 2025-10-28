#include <Core/math/math.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Render/lighting.hpp>

namespace Engine
{
	trinex_implement_engine_class(SpotLightComponent, 0)
	{
		trinex_refl_virtual_prop(m_outer_cone_angle, outer_cone_angle, outer_cone_angle)
		        ->tooltip("Outer Cone Angle of this spot light");

		trinex_refl_virtual_prop(m_inner_cone_angle, inner_cone_angle, inner_cone_angle)
		        ->tooltip("Inner Cone Angle of this spot light");
	}

	SpotLightComponent::SpotLightComponent() : m_inner_cone_angle(10.f), m_outer_cone_angle(43.f) {}

	float SpotLightComponent::calculate_light_intensity() const
	{
		if (intensity_units() == LightUnits::Lumens)
		{
			return intensity() / (2.f * Math::pi() * (1.f - Math::cos(m_outer_cone_angle)));
		}

		return Super::calculate_light_intensity();
	}

	float SpotLightComponent::inner_cone_angle() const
	{
		return m_inner_cone_angle;
	}

	float SpotLightComponent::outer_cone_angle() const
	{
		return m_outer_cone_angle;
	}

	SpotLightComponent& SpotLightComponent::inner_cone_angle(float value)
	{
		m_inner_cone_angle = Math::clamp(value, 0.f, m_outer_cone_angle);
		return *this;
	}

	SpotLightComponent& SpotLightComponent::outer_cone_angle(float value)
	{
		m_outer_cone_angle = Math::clamp(value, 0.f, 89.f);
		m_inner_cone_angle = Math::clamp(m_inner_cone_angle, 0.f, m_outer_cone_angle);
		return *this;
	}

	LightComponent::Type SpotLightComponent::light_type() const
	{
		return LightComponent::Type::Spot;
	}

	SpotLightComponent& SpotLightComponent::render_parameters(LightRenderParameters& out)
	{
		Super::render_parameters(out);

		const float outer = Math::cos(Math::radians(m_outer_cone_angle));
		const float inner = Math::cos(Math::radians(m_inner_cone_angle));

		out.spot_angles = {outer, 1.0f / (inner - outer)};
		out.direction   = direction();
		return *this;
	}
}// namespace Engine
