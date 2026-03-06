#include <Core/profiler.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/Render/primitive_context.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Graphics/mesh.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
	trinex_implement_engine_class(StaticMeshComponent, Refl::Class::IsScriptable)
	{
		trinex_refl_virtual_prop(mesh, mesh, mesh)->tooltip("Mesh object of this component");

		auto r = ScriptClassRegistrar::existing_class(static_reflection());
		r.method("StaticMesh@ mesh() const final", overload_of<StaticMesh*()>(&This::mesh));
		r.method("StaticMeshComponent@ mesh(StaticMesh@ mesh) final", overload_of<StaticMeshComponent&()>(&This::mesh));
	}

	usize StaticMeshComponent::materials_count() const
	{
		if (m_mesh)
			return m_mesh->materials.size();

		return 0;
	}

	MaterialInterface* StaticMeshComponent::material(usize index) const
	{
		if (MaterialInterface* material = Super::material(index))
			return material;

		if (mesh() && index < mesh()->materials.size())
			return mesh()->materials[index];

		return nullptr;
	}

	usize StaticMeshComponent::lods_count() const
	{
		return m_mesh->lods.size();
	}

	usize StaticMeshComponent::surfaces_count(usize lod) const
	{
		return m_mesh->lods[lod].surfaces.size();
	}

	const MeshSurface* StaticMeshComponent::surface(usize index, usize lod) const
	{
		return &m_mesh->lods[lod].surfaces[index];
	}

	const MeshVertexAttribute* StaticMeshComponent::vertex_attribute(RHIVertexSemantic semantic, usize lod)
	{
		return m_mesh->lods[lod].find_attribute(semantic);
	}

	VertexBufferBase* StaticMeshComponent::vertex_buffer(u8 stream, usize lod)
	{
		return &m_mesh->lods[lod].buffers[stream];
	}

	IndexBuffer* StaticMeshComponent::index_buffer(usize lod)
	{
		auto& buffer = m_mesh->lods[lod].indices;
		return buffer.size() == 0 ? nullptr : &buffer;
	}

	StaticMeshComponent& StaticMeshComponent::render(PrimitiveRenderingContext* ctx)
	{
		trinex_profile_cpu_n("StaticMeshComponent::render");

		static Name permutation = "StaticMesh";

		if ((ctx->pass = ctx->pass->find_permutation(permutation)))
		{
			Super::render(ctx);
		}

		return *this;
	}

	StaticMeshComponent& StaticMeshComponent::update_bounding_box()
	{
		if (mesh())
		{
			trinex_profile_cpu_n("Bounds transform");
			m_bounding_box = mesh()->bounds.transform(world_transform().matrix());
		}
		else
		{
			Super::update_bounding_box();
		}
		return *this;
	}
}// namespace Engine
