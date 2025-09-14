#include <Core/etl/templates.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/ActorComponents/skeletal_mesh_component.hpp>
#include <Engine/Actors/skeletal_mesh_actor.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
	trinex_implement_engine_class(SkeletalMeshActor, Refl::Class::IsScriptable)
	{
		auto r = ScriptClassRegistrar::existing_class(static_reflection());
		r.method("SkeletalMeshComponent@ mesh_component() const final", trinex_scoped_method(This, mesh_component));
	}

	SkeletalMeshActor::SkeletalMeshActor()
	{
		m_mesh_component = create_component<SkeletalMeshComponent>("SkeletalMeshComponent");
	}

	SkeletalMeshComponent* SkeletalMeshActor::mesh_component() const
	{
		return m_mesh_component;
	}

	SkeletalMeshActor::~SkeletalMeshActor() {}
}// namespace Engine
