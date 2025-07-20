#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Graphics/mesh.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
	trinex_implement_engine_class(StaticMeshComponent, Refl::Class::IsScriptable)
	{
		trinex_refl_virtual_prop(static_reflection(), mesh, mesh, mesh)->tooltip("Mesh object of this component");

		auto r = ScriptClassRegistrar::existing_class(static_reflection());
		r.method("StaticMesh@ mesh() const final", method_of<StaticMesh*>(&This::mesh));
		r.method("StaticMeshComponent@ mesh(StaticMesh@ mesh) final", method_of<StaticMeshComponent&, StaticMesh*>(&This::mesh));
	}

	size_t StaticMeshComponent::Proxy::lods() const
	{
		return m_mesh->lods.size();
	}

	size_t StaticMeshComponent::Proxy::materials_count() const
	{
		return m_mesh->materials.size();
	}

	size_t StaticMeshComponent::Proxy::surfaces(size_t lod) const
	{
		return m_mesh->lods[lod].surfaces.size();
	}

	const MeshSurface* StaticMeshComponent::Proxy::surface(size_t index, size_t lod) const
	{
		return &m_mesh->lods[lod].surfaces[index];
	}

	MaterialInterface* StaticMeshComponent::Proxy::material(size_t index) const
	{
		if (auto material = Super::Proxy::material(index))
			return material;

		return m_mesh ? m_mesh->materials[index] : nullptr;
	}

	VertexBufferBase* StaticMeshComponent::Proxy::find_vertex_buffer(RHIVertexBufferSemantic semantic, Index index, size_t lod)
	{
		return m_mesh->lods[lod].find_vertex_buffer(semantic, index);
	}

	IndexBuffer* StaticMeshComponent::Proxy::find_index_buffer(size_t lod)
	{
		auto& buffer = m_mesh->lods[lod].indices;
		return buffer.size() == 0 ? nullptr : &buffer;
	}

	StaticMeshComponent& StaticMeshComponent::submit_new_mesh()
	{
		render_thread()->call([proxy = proxy(), mesh = m_mesh]() { proxy->m_mesh = mesh; });
		return *this;
	}

	StaticMeshComponent::Proxy* StaticMeshComponent::create_proxy()
	{
		return new Proxy();
	}

	StaticMeshComponent& StaticMeshComponent::update_bounding_box()
	{
		if (mesh())
		{
			m_bounding_box = mesh()->bounds.apply_transform(world_transform().matrix());
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
