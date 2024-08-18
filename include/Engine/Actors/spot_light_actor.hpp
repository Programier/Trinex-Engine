#pragma once
#include <Engine/Actors/light_actor.hpp>

namespace Engine
{
	class PointLightComponent;

	class ENGINE_EXPORT SpotLightActor : public LightActor
	{
		declare_class(SpotLightActor, LightActor);

	private:
		class SpotLightComponent* m_spot_light_component = nullptr;

	public:
		SpotLightActor();
		SpotLightComponent* spot_light_component() const;
	};
}// namespace Engine
