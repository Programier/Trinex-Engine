#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/local_light_component.hpp>

namespace Engine
{
	trinex_implement_engine_class(LocalLightComponent, 0)
	{
		auto* self = static_class_instance();

		trinex_refl_prop(self, This, m_attenuation_radius)
		        ->display_name("Attenuation radius")
		        .tooltip("Attenuation radius of this light");
	}

	LocalLightComponent::LocalLightComponent() : m_attenuation_radius(30.f) {}

	LocalLightComponent& LocalLightComponent::submit_local_light_info()
	{
		render_thread()->call([proxy = proxy(), radius = m_attenuation_radius]() { proxy->m_attenuation_radius = radius; });
		return *this;
	}

	LocalLightComponent& LocalLightComponent::start_play()
	{
		Super::start_play();
		submit_local_light_info();
		return *this;
	}

	LocalLightComponent::Proxy* LocalLightComponent::create_proxy()
	{
		return new Proxy();
	}

	LocalLightComponent& LocalLightComponent::on_property_changed(const Refl::PropertyChangedEvent& event)
	{
		Super::on_property_changed(event);

		if (event.property->owner() == static_class_instance())
		{
			submit_local_light_info();
		}

		return *this;
	}
}// namespace Engine
