#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>
#include <Engine/Render/lighting.hpp>

namespace Engine
{
	trinex_implement_engine_class(PointLightComponent, 0)
	{
		trinex_refl_virtual_prop(Source Radius, source_radius, source_radius)->tooltip("Source radius of this light");
		trinex_refl_virtual_prop(Fall Off Exponent, fall_off_exponent, fall_off_exponent)
		        ->tooltip("Fall Off Exponent of this light");
	}

	PointLightComponent::Proxy& PointLightComponent::Proxy::render_parameters(LightRenderParameters& out)
	{
		Super::Proxy::render_parameters(out);
		out.source_radius     = m_source_radius;
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
		render_thread()->call([proxy = proxy(), radius = m_source_radius, exp = m_fall_off_exponent]() {
			proxy->m_source_radius     = radius;
			proxy->m_fall_off_exponent = exp;
		});
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
		return trx_new Proxy();
	}
}// namespace Engine
