#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Render/light_parameters.hpp>

namespace Engine
{
	trinex_implement_engine_class(SpotLightComponent, 0)
	{
		auto* self = static_class_instance();

		auto on_data_changed = [](const Refl::PropertyChangedEvent& event) {
			event.context_as<SpotLightComponent>()->submit_spot_light_data();
		};

		trinex_refl_prop(self, This, m_outer_cone_angle)
		        ->push_change_listener(on_data_changed)
		        .display_name("Outer Cone Angle")
		        .tooltip("Outer Cone Angle of this spot light");

		trinex_refl_prop(self, This, m_inner_cone_angle)
		        ->push_change_listener(on_data_changed)
		        .display_name("Inner Cone Angle")
		        .tooltip("Inner Cone Angle of this spot light");
	}

	SpotLightComponent::Proxy& SpotLightComponent::Proxy::update_spot_angles()
	{
		m_cos_outer_cone_angle    = glm::cos(m_outer_cone_angle);
		float cos_inner_cone      = glm::cos(m_inner_cone_angle);
		m_inv_cos_cone_difference = 1.0f / (cos_inner_cone - m_cos_outer_cone_angle);
		return *this;
	}


	SpotLightComponent::Proxy& SpotLightComponent::Proxy::render_parameters(LightRenderParameters& out)
	{
		Super::Proxy::render_parameters(out);
		out.spot_angles = {m_cos_outer_cone_angle, m_inv_cos_cone_difference};
		out.direction   = direction();
		return *this;
	}

	SpotLightComponent::SpotLightComponent() : m_inner_cone_angle(10.f), m_outer_cone_angle(43.f) {}


	SpotLightComponent& SpotLightComponent::submit_spot_light_data()
	{
		m_outer_cone_angle = glm::clamp(m_outer_cone_angle, 0.f, 89.f);
		m_inner_cone_angle = glm::clamp(m_inner_cone_angle, 0.f, m_outer_cone_angle);

		render_thread()->call([proxy = proxy(), outer_cone = m_outer_cone_angle, inner_cone = m_inner_cone_angle]() {
			proxy->m_outer_cone_angle = glm::radians(outer_cone);
			proxy->m_inner_cone_angle = glm::radians(inner_cone);
			proxy->update_spot_angles();
		});
		return *this;
	}

	float SpotLightComponent::inner_cone_angle() const
	{
		return m_inner_cone_angle;
	}

	float SpotLightComponent::outer_cone_angle() const
	{
		return m_outer_cone_angle;
	}

	SpotLightComponent& SpotLightComponent::inner_cone_angle(float value)
	{
		m_inner_cone_angle = value;
		return submit_spot_light_data();
	}

	SpotLightComponent& SpotLightComponent::outer_cone_angle(float value)
	{
		m_outer_cone_angle = value;
		return submit_spot_light_data();
	}

	LightComponent::Type SpotLightComponent::light_type() const
	{
		return LightComponent::Type::Spot;
	}

	SpotLightComponent& SpotLightComponent::start_play()
	{
		Super::start_play();
		submit_spot_light_data();
		return *this;
	}

	SpotLightComponent::Proxy* SpotLightComponent::create_proxy()
	{
		return new Proxy();
	}
}// namespace Engine
