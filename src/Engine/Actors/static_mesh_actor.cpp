#include <Core/etl/templates.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/Actors/static_mesh_actor.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
	implement_engine_class(StaticMeshActor, Refl::Class::IsScriptable)
	{
		auto r = ScriptClassRegistrar::existing_class(static_class_instance());
		r.method("StaticMeshComponent@ mesh_component() const final", trinex_scoped_method(This, mesh_component));
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
