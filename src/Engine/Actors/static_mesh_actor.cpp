#include <Core/class.hpp>
#include <Core/property.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/Actors/static_mesh_actor.hpp>

namespace Engine
{
	implement_engine_class(StaticMeshActor, 0)
	{
		Class* self = This::static_class_instance();
		self->add_property(new ObjectProperty("StaticMeshComponent", "StaticMeshComponent", &This::m_mesh_component));
	}

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
