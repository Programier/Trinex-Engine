#pragma once

#include <Engine/ActorComponents/point_light_component.hpp>

namespace Engine
{
	class ENGINE_EXPORT SpotLightComponentProxy : public PointLightComponentProxy
	{
		float m_inner_cone_angle;
		float m_outer_cone_angle;
		float m_cos_outer_cone_angle;
		float m_inv_cos_cone_difference;

	private:
		SpotLightComponentProxy& update_spot_angles();

	public:
		float inner_cone_angle() const;
		float outer_cone_angle() const;
		float cos_outer_cone_angle() const;
		float inv_cos_cone_difference() const;

		SpotLightComponentProxy& inner_cone_angle(float value);
		SpotLightComponentProxy& outer_cone_angle(float value);
		Vector3f direction() const;

		friend class SpotLightComponent;
	};

	class ENGINE_EXPORT SpotLightComponent : public PointLightComponent
	{
		trinex_declare_class(SpotLightComponent, PointLightComponent);

	private:
		float m_inner_cone_angle;
		float m_outer_cone_angle;

		SpotLightComponent& submit_spot_light_data();

	public:
		SpotLightComponent();

		float inner_cone_angle() const;
		float outer_cone_angle() const;
		SpotLightComponent& inner_cone_angle(float value);
		SpotLightComponent& outer_cone_angle(float value);

		Vector3f direction() const;
		SpotLightComponentProxy* proxy() const;

		Type light_type() const override;
		SpotLightComponent& start_play() override;
		SpotLightComponent& render(class SceneRenderer*) override;
		ActorComponentProxy* create_proxy() override;
	};

}// namespace Engine
