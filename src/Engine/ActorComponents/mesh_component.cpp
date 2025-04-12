#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/ActorComponents/mesh_component.hpp>
#include <Graphics/material.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
	trinex_implement_engine_class(MeshComponent, Refl::Class::IsScriptable)
	{
		trinex_refl_prop(static_class_instance(), This, material_overrides)->tooltip("Material overrides of this component");
	}
}// namespace Engine
