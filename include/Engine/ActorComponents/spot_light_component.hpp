#pragma once
#include <Engine/ActorComponents/point_light_component.hpp>

namespace Engine
{
	class ENGINE_EXPORT SpotLightComponent : public PointLightComponent
	{
		trinex_declare_class(SpotLightComponent, PointLightComponent);

	public:
		class ENGINE_EXPORT Proxy : public Super::Proxy
		{
			float m_inner_cone_angle;
			float m_outer_cone_angle;
			float m_cos_outer_cone_angle;
			float m_inv_cos_cone_difference;

		private:
			Proxy& update_spot_angles();

		public:
			inline float inner_cone_angle() const { return m_inner_cone_angle; }
			inline float outer_cone_angle() const { return m_outer_cone_angle; }
			inline float cos_outer_cone_angle() const { return m_cos_outer_cone_angle; }
			inline float inv_cos_cone_difference() const { return m_inv_cos_cone_difference; }
			inline Vector3f direction() const { return world_transform().forward_vector(); }
			Proxy& render_parameters(LightRenderParameters& out) override;
			Type light_type() const override;

			friend class SpotLightComponent;
		};

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

		Type light_type() const override;
		SpotLightComponent& start_play() override;
		Proxy* create_proxy() override;

		inline Vector3f direction() const { return world_transform().forward_vector(); }
		inline Proxy* proxy() const { return typed_proxy<Proxy>(); }
	};

}// namespace Engine
