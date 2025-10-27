#pragma once
#include <Engine/ActorComponents/light_component.hpp>

namespace Engine
{
	class ENGINE_EXPORT DirectionalLightComponent : public LightComponent
	{
		trinex_declare_class(DirectionalLightComponent, LightComponent);

	public:
		class ENGINE_EXPORT Proxy : public Super::Proxy
		{
		private:
			float m_shadows_distance = 50.f;

		public:
			inline Vector3f direction() const { return world_transform().forward_vector(); }
			inline float shadows_distance() const { return m_shadows_distance; }
			Type light_type() const override;
			Proxy& render_parameters(LightRenderParameters& out) override;
			friend class DirectionalLightComponent;
		};

	private:
		float m_shadows_distance = 50.f;
		
	protected:
		float calculate_light_intensity() const override;

	public:
		Type light_type() const override;
		Proxy* create_proxy() override;
		DirectionalLightComponent& shadows_distance(float value);

		inline Vector3f direction() const { return world_transform().forward_vector(); }
		inline float shadows_distance() const { return m_shadows_distance; }
		inline Proxy* proxy() const { return typed_proxy<Proxy>(); }
	};
}// namespace Engine
