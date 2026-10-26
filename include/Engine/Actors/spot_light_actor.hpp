#pragma once
#include <Engine/Actors/light_actor.hpp>

namespace Trinex
{
	class PointLightComponent;

	class trinex_api SpotLightActor : public LightActor
	{
		trinex_class(SpotLightActor, LightActor);

	private:
		class SpotLightComponent* m_spot_light_component = nullptr;

	public:
		SpotLightActor();
		SpotLightComponent* spot_light_component() const;
	};
} // namespace Trinex
