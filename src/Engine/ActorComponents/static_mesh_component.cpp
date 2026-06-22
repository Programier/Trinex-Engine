#include <Core/profiler.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/Render/scene.hpp>
#include <Graphics/mesh.hpp>
#include <RHI/handles.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Trinex
{
	static RHIDescriptor buffer_descriptor(const VertexBufferBase& buffer)
	{
		return buffer.handle() ? buffer.handle()->as_srv(RHIBufferViewType::ByteAddress)->descriptor() : 0;
	}

	static RHIDescriptor buffer_descriptor(const IndexBuffer& buffer)
	{
		return buffer.handle() ? buffer.handle()->as_srv(RHIBufferViewType::ByteAddress)->descriptor() : 0;
	}

	static u32 create_geometry(RenderScene* scene, StaticMesh* owner, StaticMesh::LOD& mesh)
	{
		RenderScene::Geometry geometry;

		geometry.vertex_stream.buffer = buffer_descriptor(mesh.vertex_stream);
		geometry.vertex_stream.stride = sizeof(MeshVertexStream);

		geometry.surface_stream.buffer = buffer_descriptor(mesh.surface_stream);
		geometry.surface_stream.stride = sizeof(MeshSurfaceStream);

		geometry.index_stream.buffer = buffer_descriptor(mesh.indices);
		geometry.index_stream.stride = mesh.indices.stride();

		geometry.aabb = owner->bounds;

		return scene->create_geometry(geometry);
	}


	trinex_implement_engine_class(StaticMeshComponent, Refl::Class::IsScriptable)
	{
		trinex_refl_virtual_prop(mesh, mesh, mesh)->tooltip("Mesh object of this component");

		auto r = ScriptClassRegistrar::existing_class(static_reflection());
		r.method("StaticMesh@ mesh() const final", overload_of<StaticMesh*()>(&This::mesh));
		r.method("StaticMeshComponent@ mesh(StaticMesh@ mesh) final", overload_of<StaticMeshComponent&()>(&This::mesh));
	}

	MaterialInterface* StaticMeshComponent::material(usize index) const
	{
		if (MaterialInterface* material = Super::material(index))
			return material;

		if (mesh() && index < mesh()->materials.size())
			return mesh()->materials[index];

		return nullptr;
	}

	StaticMeshComponent& StaticMeshComponent::start_play()
	{
		Super::start_play();

		if (m_mesh == nullptr)
			return *this;

		auto& lod        = m_mesh->lods[0];
		usize primitives = lod.surfaces.size();
		Matrix4f matrix  = world_transform().matrix();

		m_transform = scene()->allocate(sizeof(Matrix4f), &matrix);
		m_geometry  = create_geometry(scene(), m_mesh, lod);
		StackByteAllocator::Mark mark;

		auto descriptions = StackAllocator<RenderScene::Primitive>::allocate(primitives);
		{
			for (usize i = 0; i < primitives; ++i)
			{
				auto& surface = lod.surfaces[i];

				descriptions[i] = {
				        .material       = m_mesh->materials[surface.material_index],
				        .first_vertex   = surface.first_vertex,
				        .first_index    = surface.first_index,
				        .vertices_count = surface.vertices_count,
				        .transform      = m_transform,
				        .data           = 0,
				        .geometry       = m_geometry,
				        .flags          = 0,
				};
			}
		}
		m_primitive = scene()->create_primitive(descriptions, primitives);
		return *this;
	}

	StaticMeshComponent& StaticMeshComponent::stop_play()
	{
		if (RenderScene* render_scene = scene())
		{
			render_scene->free(m_transform);
			render_scene->release_geometry(m_geometry);
			render_scene->release_primitive(m_primitive);
		}

		Super::despawned();
		return *this;
	}

	StaticMeshComponent& StaticMeshComponent::on_transform_changed()
	{
		Super::on_transform_changed();

		if (m_transform)
		{
			Matrix4f matrix = world_transform().matrix();
			scene()->update(m_transform, &matrix, sizeof(matrix));
		}
		return *this;
	}

}// namespace Trinex
