#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/local_light_component.hpp>
#include <Engine/Render/lighting.hpp>

namespace Engine
{
	trinex_implement_engine_class(LocalLightComponent, 0)
	{
		trinex_refl_prop(m_attenuation_radius)->display_name("Attenuation radius").tooltip("Attenuation radius of this light");
	}

	LocalLightComponent::LocalLightComponent() : m_attenuation_radius(30.f) {}

	LocalLightComponent& LocalLightComponent::on_property_changed(const Refl::PropertyChangedEvent& event)
	{
		Super::on_property_changed(event);

		if (event.property->owner() == static_reflection())
		{
			on_transform_changed();
		}

		return *this;
	}

	LocalLightComponent& LocalLightComponent::update_bounding_box()
	{
		const Vector3f& location = world_transform().location;
		Vector3f extends         = {m_attenuation_radius, m_attenuation_radius, m_attenuation_radius};
		m_bounding_box           = Box3f(location - extends, location + extends);
		return *this;
	}

	LocalLightComponent& LocalLightComponent::render_parameters(LightRenderParameters& out)
	{
		Super::render_parameters(out);
		out.attenuation_radius     = m_attenuation_radius;
		out.inv_attenuation_radius = 1.f / m_attenuation_radius;
		return *this;
	}
}// namespace Engine
