#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>
#include <Engine/Render/light_parameters.hpp>

namespace Engine
{
	trinex_implement_engine_class(PointLightComponent, 0)
	{
		auto* self = static_class_instance();

		trinex_refl_prop(self, This, m_fall_off_exponent)
		        ->display_name("Fall Off Exponent")
		        .tooltip("Fall Off Exponent of this light");
	}

	PointLightComponent::Proxy& PointLightComponent::Proxy::render_parameters(LightRenderParameters& out)
	{
		Super::Proxy::render_parameters(out);
		out.fall_off_exponent = m_fall_off_exponent;
		return *this;
	}

	PointLightComponent::Type PointLightComponent::Proxy::light_type() const
	{
		return Point;
	}

	PointLightComponent::PointLightComponent() : m_fall_off_exponent(2.f) {}

	PointLightComponent& PointLightComponent::submit_point_light_data()
	{
		render_thread()->create_task<UpdateVariableCommand<float>>(m_fall_off_exponent, proxy()->m_fall_off_exponent);
		return *this;
	}

	PointLightComponent& PointLightComponent::start_play()
	{
		Super::start_play();
		submit_point_light_data();
		return *this;
	}

	PointLightComponent::Proxy* PointLightComponent::create_proxy()
	{
		return new Proxy();
	}

	PointLightComponent& PointLightComponent::on_property_changed(const Refl::PropertyChangedEvent& event)
	{
		Super::on_property_changed(event);

		if (event.property->owner() == static_class_instance())
		{
			submit_point_light_data();
		}
		return *this;
	}
}// namespace Engine
