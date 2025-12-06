#pragma once
#include <Core/math/angle.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>

namespace Engine
{
	class ENGINE_EXPORT SpotLightComponent : public PointLightComponent
	{
		trinex_class(SpotLightComponent, PointLightComponent);

	private:
		Angle m_inner_cone_angle;
		Angle m_outer_cone_angle;

	protected:
		float calculate_light_intensity() const override;

	public:
		SpotLightComponent();

		SpotLightComponent& inner_cone_angle(Angle value);
		SpotLightComponent& outer_cone_angle(Angle value);

		Type light_type() const override;
		SpotLightComponent& render_parameters(LightRenderParameters& out) override;

		inline Vector3f direction() const { return world_transform().forward_vector(); }
		inline Angle inner_cone_angle() const { return m_inner_cone_angle; }
		inline Angle outer_cone_angle() const { return m_outer_cone_angle; }
	};

}// namespace Engine
