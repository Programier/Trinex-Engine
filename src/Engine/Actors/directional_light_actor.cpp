#include <Core/reflection/class.hpp>
#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/Actors/directional_light_actor.hpp>

namespace Engine
{
	trinex_implement_engine_class_default_init(DirectionalLightActor, 0);

	DirectionalLightActor::DirectionalLightActor()
	{
		m_directional_light_component = new_instance<DirectionalLightComponent>("DirectionalLightComponent", this);
	}

	DirectionalLightComponent* DirectionalLightActor::directional_light_component() const
	{
		return m_directional_light_component;
	}
}// namespace Engine
