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
		r.method("StaticMesh@ mesh() const final", method_of<StaticMesh*>(&This::mesh));
		r.method("StaticMeshComponent@ mesh(StaticMesh@ mesh) final", method_of<StaticMeshComponent&, StaticMesh*>(&This::mesh));
	}

	size_t StaticMeshComponent::Proxy::lods_count() const
	{
		return m_mesh->lods.size();
	}

	size_t StaticMeshComponent::Proxy::materials_count() const
	{
		return m_mesh->materials.size();
	}

	size_t StaticMeshComponent::Proxy::surfaces_count(size_t lod) const
	{
		return m_mesh->lods[lod].surfaces.size();
	}

	const MeshSurface* StaticMeshComponent::Proxy::surface(size_t index, size_t lod) const
	{
		return &m_mesh->lods[lod].surfaces[index];
	}

	VertexBufferBase* StaticMeshComponent::Proxy::vertex_buffer(byte stream, size_t lod)
	{
		return &m_mesh->lods[lod].buffers[stream];
	}

	IndexBuffer* StaticMeshComponent::Proxy::index_buffer(size_t lod)
	{
		auto& buffer = m_mesh->lods[lod].indices;
		return buffer.size() == 0 ? nullptr : &buffer;
	}

	MaterialInterface* StaticMeshComponent::Proxy::material(size_t index) const
	{
		if (auto material = Super::Proxy::material(index))
			return material;

		return m_mesh ? m_mesh->materials[index] : nullptr;
	}

	StaticMeshComponent::Proxy& StaticMeshComponent::Proxy::render(PrimitiveRenderingContext* ctx)
	{
		static Name permutation = "StaticMesh";

		if ((ctx->pass = ctx->pass->find_permutation(permutation)))
		{
			Super::Proxy::render(ctx);
		}

		return *this;
	}

	StaticMeshComponent& StaticMeshComponent::submit_new_mesh()
	{
		render_thread()->call([proxy = proxy(), mesh = m_mesh]() { proxy->m_mesh = mesh; });
		update_bounding_box();
		return *this;
	}

	StaticMeshComponent::Proxy* StaticMeshComponent::create_proxy()
	{
		return trx_new Proxy();
	}

	StaticMeshComponent& StaticMeshComponent::update_bounding_box()
	{
		if (mesh())
		{
			m_bounding_box = mesh()->bounds.transform(world_transform().matrix());
			submit_bounds_to_render_thread();
		}
		else
		{
			Super::update_bounding_box();
		}
		return *this;
	}

	size_t StaticMeshComponent::materials_count() const
	{
		if (m_mesh)
			return m_mesh->materials.size();

		return 0;
	}

	MaterialInterface* StaticMeshComponent::material(size_t index) const
	{
		if (MaterialInterface* material = Super::material(index))
			return material;

		if (mesh() && index < mesh()->materials.size())
			return mesh()->materials[index];

		return nullptr;
	}
}// namespace Engine
