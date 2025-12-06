#pragma once
#include <Core/constants.hpp>
#include <Core/pointer.hpp>
#include <Engine/ActorComponents/mesh_component.hpp>

namespace Engine
{
	class RHIBuffer;

	class ENGINE_EXPORT SkeletalMeshComponent : public MeshComponent
	{
		trinex_class(SkeletalMeshComponent, MeshComponent);

	private:
		class SkeletalMesh* m_mesh = nullptr;
		RHIBuffer* m_bones         = nullptr;

	public:
		SkeletalMeshComponent& update_bounding_box() override;

		using MeshComponent::material;

		MaterialInterface* material(size_t index) const override;
		size_t lods_count() const override;
		size_t materials_count() const override;
		size_t surfaces_count(size_t lod = 0) const override;
		const MeshSurface* surface(size_t index, size_t lod = 0) const override;
		const MeshVertexAttribute* vertex_attribute(RHIVertexSemantic semantic, size_t lod = 0) override;
		VertexBufferBase* vertex_buffer(byte stream, size_t lod = 0) override;
		IndexBuffer* index_buffer(size_t lod = 0) override;
		SkeletalMeshComponent& render(PrimitiveRenderingContext* context) override;

		inline SkeletalMesh* mesh() const { return m_mesh; }

		SkeletalMeshComponent& mesh(SkeletalMesh* mesh)
		{
			m_mesh = mesh;
			return update_bounding_box();
		}
	};
}// namespace Engine
