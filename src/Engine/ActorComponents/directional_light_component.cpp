#include <Core/reflection/class.hpp>
#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/Render/light_parameters.hpp>

namespace Engine
{
	trinex_implement_engine_class_default_init(DirectionalLightComponent, 0);

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
}// namespace Engine
