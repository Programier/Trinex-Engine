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

namespace Engine
{
	trinex_implement_engine_class(SkeletalMeshComponent, Refl::Class::IsScriptable)
	{
		trinex_refl_virtual_prop(mesh, mesh, mesh)->tooltip("Mesh object of this component");

		auto r = ScriptClassRegistrar::existing_class(static_reflection());
		r.method("SkeletalMesh@ mesh() const final", method_of<SkeletalMesh*>(&This::mesh));
		r.method("SkeletalMeshComponent@ mesh(SkeletalMesh@ mesh) final",
		         method_of<SkeletalMeshComponent&, SkeletalMesh*>(&This::mesh));
	}

	SkeletalMeshComponent& SkeletalMeshComponent::update_bounding_box()
	{
		if (mesh())
		{
			m_bounding_box = mesh()->bounds.transform(world_transform().matrix());
		}
		else
		{
			Super::update_bounding_box();
		}
		return *this;
	}

	size_t SkeletalMeshComponent::materials_count() const
	{
		if (m_mesh)
			return m_mesh->materials.size();

		return 0;
	}

	MaterialInterface* SkeletalMeshComponent::material(size_t index) const
	{
		if (MaterialInterface* material = Super::material(index))
			return material;

		if (mesh() && index < mesh()->materials.size())
			return mesh()->materials[index];

		return nullptr;
	}

	size_t SkeletalMeshComponent::lods_count() const
	{
		return m_mesh->lods.size();
	}

	size_t SkeletalMeshComponent::surfaces_count(size_t lod) const
	{
		return m_mesh->lods[lod].surfaces.size();
	}

	const MeshSurface* SkeletalMeshComponent::surface(size_t index, size_t lod) const
	{
		return &m_mesh->lods[lod].surfaces[index];
	}

	const MeshVertexAttribute* SkeletalMeshComponent::vertex_attribute(RHIVertexSemantic semantic, size_t lod)
	{
		return nullptr;
	}

	VertexBufferBase* SkeletalMeshComponent::vertex_buffer(byte stream, size_t lod)
	{
		return nullptr;
	}

	IndexBuffer* SkeletalMeshComponent::index_buffer(size_t lod)
	{
		auto& buffer = m_mesh->lods[lod].indices;
		return buffer.size() == 0 ? nullptr : &buffer;
	}

	SkeletalMeshComponent& SkeletalMeshComponent::render(PrimitiveRenderingContext* ctx)
	{
		static MaterialBindings skeletal_bindings;
		static Name permutation                              = "SkeletalMesh";
		static MaterialBindings::Binding* trx_skinning_bones = skeletal_bindings.find_or_create("trx_skinning_bones");

		if ((ctx->pass = ctx->pass->find_permutation(permutation)))
		{
			skeletal_bindings.prev = ctx->bindings;
			ctx->bindings          = &skeletal_bindings;

			(*trx_skinning_bones) = m_bones->as_srv();
			Super::render(ctx);
		}
		return *this;
	}
}// namespace Engine
