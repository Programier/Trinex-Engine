#include <Core/reflection/class.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Actors/spot_light_actor.hpp>
#include <Graphics/texture.hpp>

namespace Engine
{
	trinex_implement_engine_class_default_init(SpotLightActor, 0);

	SpotLightActor::SpotLightActor()
	{
		m_spot_light_component = new_instance<SpotLightComponent>("SpotLightComponent", this);
	}

	SpotLightComponent* SpotLightActor::spot_light_component() const
	{
		return m_spot_light_component;
	}
}// namespace Engine
