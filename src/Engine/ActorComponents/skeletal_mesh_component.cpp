#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/skeletal_mesh_component.hpp>
#include <Engine/Render/primitive_context.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Graphics/material_bindings.hpp>
#include <Graphics/mesh.hpp>
#include <RHI/handles.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Trinex
{
	trinex_implement_engine_class(SkeletalMeshComponent, Refl::Class::IsScriptable)
	{
		trinex_refl_virtual_prop(mesh, mesh, mesh)->tooltip("Mesh object of this component");

		auto r = ScriptClassRegistrar::existing_class(static_reflection());
		r.method("SkeletalMesh@ mesh() const final", overload_of<SkeletalMesh*()>(&This::mesh));
		r.method("SkeletalMeshComponent@ mesh(SkeletalMesh@ mesh) final", overload_of<SkeletalMeshComponent&()>(&This::mesh));
	}

	MaterialInterface* SkeletalMeshComponent::material(usize index) const
	{
		if (MaterialInterface* material = Super::material(index))
			return material;

		if (mesh() && index < mesh()->materials.size())
			return mesh()->materials[index];

		return nullptr;
	}
}// namespace Trinex
