#include <Core/reflection/class.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>
#include <Engine/Actors/point_light_actor.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine
{
	trinex_implement_engine_class_default_init(PointLightActor, 0);

	PointLightActor::PointLightActor()
	{
		m_point_light_component = create_component<PointLightComponent>(("PointLightComponent"));
	}

	PointLightComponent* PointLightActor::point_light_component() const
	{
		return m_point_light_component;
	}
}// namespace Engine
