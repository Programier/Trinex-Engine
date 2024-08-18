#pragma once
#include <Engine/Actors/light_actor.hpp>

namespace Engine
{
	class PointLightComponent;

	class ENGINE_EXPORT PointLightActor : public LightActor
	{
		declare_class(PointLightActor, LightActor);

	private:
		class PointLightComponent* m_point_light_component = nullptr;

	public:
		PointLightActor();
		PointLightComponent* point_light_component() const;
	};
}// namespace Engine
