#pragma once
#include <Engine/Actors/light_actor.hpp>

namespace Trinex
{
	class PointLightComponent;

	class trinex_api PointLightActor : public LightActor
	{
		trinex_class(PointLightActor, LightActor);

	private:
		class PointLightComponent* m_point_light_component = nullptr;

	public:
		PointLightActor();
		PointLightComponent* point_light_component() const;
	};
} // namespace Trinex
