#pragma once
#include <Engine/ActorComponents/light_component.hpp>

namespace Engine
{
	class ENGINE_EXPORT LocalLightComponentProxy : public LightComponentProxy
	{
	private:
		float m_attenuation_radius;

	public:
		float attenuation_radius() const;
		LocalLightComponentProxy& attenuation_radius(float value);
		friend class LocalLightComponent;
	};

	class ENGINE_EXPORT LocalLightComponent : public LightComponent
	{
		trinex_declare_class(LocalLightComponent, LightComponent);

	private:
		float m_attenuation_radius;

		LocalLightComponent& submit_local_light_info();

	public:
		LocalLightComponent();
		float attenuation_radius() const;
		LocalLightComponentProxy* proxy() const;
		LocalLightComponent& attenuation_radius(float value);
		LocalLightComponent& start_play() override;
		ActorComponentProxy* create_proxy() override;
		LocalLightComponent& render(class SceneRenderer*) override;
		LocalLightComponent& on_property_changed(const Refl::PropertyChangedEvent& event) override;
	};
}// namespace Engine
