#pragma once
#include <Engine/ActorComponents/light_component.hpp>

namespace Engine
{
	class ENGINE_EXPORT LocalLightComponent : public LightComponent
	{
		trinex_declare_class(LocalLightComponent, LightComponent);

	public:
		class ENGINE_EXPORT Proxy : public Super::Proxy
		{
		private:
			float m_attenuation_radius;

		public:
			inline float attenuation_radius() const { return m_attenuation_radius; }
			Proxy& render_parameters(LightRenderParameters& out) override;
			friend class LocalLightComponent;
		};

	private:
		float m_attenuation_radius;

		LocalLightComponent& submit_local_light_info();

	public:
		LocalLightComponent();

		LocalLightComponent& start_play() override;
		LocalLightComponent& on_property_changed(const Refl::PropertyChangedEvent& event) override;
		LocalLightComponent& update_bounding_box() override;

		inline float attenuation_radius() const { return m_attenuation_radius; }
		inline Proxy* proxy() const { return typed_proxy<Proxy>(); }

		inline LocalLightComponent& attenuation_radius(float value)
		{
			m_attenuation_radius = value;
			return submit_local_light_info();
		}
	};
}// namespace Engine
