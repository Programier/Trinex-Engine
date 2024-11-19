#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/Actors/static_mesh_actor.hpp>

namespace Engine
{
	implement_engine_class(StaticMeshActor, 0)
	{}

	StaticMeshActor::StaticMeshActor()
	{
		m_mesh_component = create_component<StaticMeshComponent>("StaticMeshComponent");
	}

	StaticMeshComponent* StaticMeshActor::mesh_component() const
	{
		return m_mesh_component;
	}

	StaticMeshActor::~StaticMeshActor()
	{}
}// namespace Engine
