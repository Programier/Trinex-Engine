#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/local_light_component.hpp>
#include <Engine/Render/scene_renderer.hpp>

namespace Engine
{
	float LocalLightComponentProxy::attenuation_radius() const
	{
		return m_attenuation_radius;
	}

	LocalLightComponentProxy& LocalLightComponentProxy::attenuation_radius(float value)
	{
		m_attenuation_radius = value;
		return *this;
	}

	implement_engine_class(LocalLightComponent, 0)
	{
		auto* self = static_class_instance();

		trinex_refl_prop(self, This, m_attenuation_radius)
				->display_name("Attenuation radius")
				.tooltip("Attenuation radius of this light");
	}

	LocalLightComponent::LocalLightComponent() : m_attenuation_radius(30.f)
	{}

	LocalLightComponent& LocalLightComponent::submit_local_light_info()
	{
		render_thread()->create_task<UpdateVariableCommand<float>>(m_attenuation_radius, proxy()->m_attenuation_radius);
		return *this;
	}

	float LocalLightComponent::attenuation_radius() const
	{
		return m_attenuation_radius;
	}

	LocalLightComponent& LocalLightComponent::attenuation_radius(float value)
	{
		m_attenuation_radius = value;
		return submit_local_light_info();
	}

	LocalLightComponent& LocalLightComponent::start_play()
	{
		Super::start_play();
		submit_local_light_info();
		return *this;
	}

	ActorComponentProxy* LocalLightComponent::create_proxy()
	{
		return new LocalLightComponentProxy();
	}

	LocalLightComponentProxy* LocalLightComponent::proxy() const
	{
		return typed_proxy<LocalLightComponentProxy>();
	}

	LocalLightComponent& LocalLightComponent::render(class SceneRenderer* renderer)
	{
		renderer->render_component(this);
		return *this;
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

	SceneRenderer& SceneRenderer::render_component(LocalLightComponent* component)
	{
		render_base_component(component);
		return *this;
	}
}// namespace Engine
