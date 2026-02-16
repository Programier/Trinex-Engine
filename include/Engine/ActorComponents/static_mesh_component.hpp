#pragma once
#include <Core/constants.hpp>
#include <Core/pointer.hpp>
#include <Engine/ActorComponents/mesh_component.hpp>
#include <Graphics/gpu_buffers.hpp>


namespace Engine
{
	class ENGINE_EXPORT StaticMeshComponent : public MeshComponent
	{
		trinex_class(StaticMeshComponent, MeshComponent);

	private:
		class StaticMesh* m_mesh = nullptr;

	public:
		using MeshComponent::material;

		MaterialInterface* material(size_t index) const override;
		size_t lods_count() const override;
		size_t materials_count() const override;
		size_t surfaces_count(size_t lod = 0) const override;
		const MeshSurface* surface(size_t index, size_t lod = 0) const override;
		const MeshVertexAttribute* vertex_attribute(RHIVertexSemantic semantic, size_t lod = 0) override;
		VertexBufferBase* vertex_buffer(byte stream, size_t lod = 0) override;
		IndexBuffer* index_buffer(size_t lod = 0) override;
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
}// namespace Engine
