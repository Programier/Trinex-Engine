#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/Render/lighting.hpp>

namespace Engine
{
	trinex_implement_engine_class(DirectionalLightComponent, 0)
	{
		trinex_refl_virtual_prop(shadows_distance, shadows_distance, shadows_distance);
	}

	DirectionalLightComponent::Type DirectionalLightComponent::Proxy::light_type() const
	{
		return Directional;
	}

	DirectionalLightComponent::Type DirectionalLightComponent::light_type() const
	{
		return Type::Directional;
	}

	DirectionalLightComponent::Proxy& DirectionalLightComponent::Proxy::render_parameters(LightRenderParameters& out)
	{
		Super::Proxy::render_parameters(out);
		out.direction = direction();
		return *this;
	}

	DirectionalLightComponent::Proxy* DirectionalLightComponent::create_proxy()
	{
		return new Proxy();
	}

	DirectionalLightComponent& DirectionalLightComponent::shadows_distance(float value)
	{
		m_shadows_distance = value;
		render_thread()->call([proxy = proxy(), value]() { proxy->m_shadows_distance = value; });
		return *this;
	}
}// namespace Engine
