#pragma once
#include <Core/constants.hpp>
#include <Core/pointer.hpp>
#include <Engine/ActorComponents/mesh_component.hpp>
#include <Graphics/gpu_buffers.hpp>


namespace Trinex
{
	class ENGINE_EXPORT StaticMeshComponent : public MeshComponent
	{
		trinex_class(StaticMeshComponent, MeshComponent);

	private:
		class StaticMesh* m_mesh = nullptr;

	public:
		using MeshComponent::material;

		MaterialInterface* material(usize index) const override;
		usize lods_count() const override;
		usize materials_count() const override;
		usize surfaces_count(usize lod = 0) const override;
		const MeshSurface* surface(usize index, usize lod = 0) const override;
		const MeshVertexAttribute* vertex_attribute(RHIVertexSemantic semantic, usize lod = 0) override;
		VertexBufferBase* vertex_buffer(u8 stream, usize lod = 0) override;
		IndexBuffer* index_buffer(usize lod = 0) override;
		StaticMeshComponent& render(PrimitiveRenderingContext* ctx) override;
		StaticMeshComponent& update_bounding_box() override;

		inline StaticMesh* mesh() const { return m_mesh; }

		inline StaticMeshComponent& mesh(StaticMesh* mesh)
		{
			m_mesh = mesh;
			on_transform_changed();
			return *this;
		}
	};
}// namespace Trinex
