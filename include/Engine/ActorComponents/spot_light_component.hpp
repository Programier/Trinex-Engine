#pragma once
#include <Engine/ActorComponents/point_light_component.hpp>

namespace Engine
{
	class ENGINE_EXPORT SpotLightComponent : public PointLightComponent
	{
		trinex_declare_class(SpotLightComponent, PointLightComponent);

	private:
		float m_inner_cone_angle;
		float m_outer_cone_angle;

	protected:
		float calculate_light_intensity() const override;

	public:
		SpotLightComponent();

		float inner_cone_angle() const;
		float outer_cone_angle() const;
		SpotLightComponent& inner_cone_angle(float value);
		SpotLightComponent& outer_cone_angle(float value);

		Type light_type() const override;
		SpotLightComponent& render_parameters(LightRenderParameters& out) override;

		inline Vector3f direction() const { return world_transform().forward_vector(); }
	};

}// namespace Engine
