#pragma once
#include <Engine/ActorComponents/local_light_component.hpp>

namespace Engine
{
	class ENGINE_EXPORT PointLightComponent : public LocalLightComponent
	{
		trinex_declare_class(PointLightComponent, LocalLightComponent);

	private:
		float m_source_radius;
		float m_fall_off_exponent;

	protected:
		float calculate_light_intensity() const override;

	public:
		PointLightComponent();

		PointLightComponent& render_parameters(LightRenderParameters& out) override;

		inline float source_radius() const { return m_source_radius; }
		inline float fall_off_exponent() const { return m_fall_off_exponent; }
		inline Type light_type() const override { return Type::Point; }

		inline LocalLightComponent& source_radius(float value)
		{
			m_source_radius = value;
			return *this;
		}

		inline PointLightComponent& fall_off_exponent(float value)
		{
			m_fall_off_exponent = value;
			return *this;
		}
	};

}// namespace Engine
