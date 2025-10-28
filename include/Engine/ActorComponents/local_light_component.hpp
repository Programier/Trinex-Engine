#pragma once
#include <Engine/ActorComponents/light_component.hpp>

namespace Engine
{
	class ENGINE_EXPORT LocalLightComponent : public LightComponent
	{
		trinex_declare_class(LocalLightComponent, LightComponent);

	private:
		float m_attenuation_radius;

	public:
		LocalLightComponent();

		LocalLightComponent& on_property_changed(const Refl::PropertyChangedEvent& event) override;
		LocalLightComponent& update_bounding_box() override;
		LocalLightComponent& render_parameters(LightRenderParameters& out) override;

		inline float attenuation_radius() const { return m_attenuation_radius; }

		inline LocalLightComponent& attenuation_radius(float value)
		{
			m_attenuation_radius = value;
			return *this;
		}
	};
}// namespace Engine
