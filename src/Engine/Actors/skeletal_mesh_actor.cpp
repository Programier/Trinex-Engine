#include <Core/etl/templates.hpp>
#include <Core/reflection/deprecated/class.hpp>
#include <Core/reflection/deprecated/property.hpp>
#include <Engine/ActorComponents/skeletal_mesh_component.hpp>
#include <Engine/Actors/skeletal_mesh_actor.hpp>
#include <ScriptEngine/script_binding.hpp>

namespace Trinex
{
	trinex_implement_engine_class(SkeletalMeshActor, Refl::Deprecated::Class::IsScriptable)
	{
		auto r = ScriptBinding::Class::existing(static_reflection());
		r.method("SkeletalMeshComponent@ mesh_component() const final", trinex_scoped_method(This, mesh_component));
	}

	SkeletalMeshActor::SkeletalMeshActor()
	{
		m_mesh_component = new_instance<SkeletalMeshComponent>("SkeletalMeshComponent", this);
	}

	SkeletalMeshComponent* SkeletalMeshActor::mesh_component() const
	{
		return m_mesh_component;
	}

	SkeletalMeshActor::~SkeletalMeshActor() {}
} // namespace Trinex
