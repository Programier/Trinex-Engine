#pragma once
#include <Core/constants.hpp>
#include <Core/pointer.hpp>
#include <Engine/ActorComponents/mesh_component.hpp>

namespace Trinex
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

		MaterialInterface* material(usize index) const override;
		usize lods_count() const override;
		usize materials_count() const override;
		usize surfaces_count(usize lod = 0) const override;
		const MeshSurface* surface(usize index, usize lod = 0) const override;
		const MeshVertexAttribute* vertex_attribute(RHIVertexSemantic semantic, usize lod = 0) override;
		VertexBufferBase* vertex_buffer(u8 stream, usize lod = 0) override;
		IndexBuffer* index_buffer(usize lod = 0) override;
		SkeletalMeshComponent& render(PrimitiveRenderingContext* context) override;

		inline SkeletalMesh* mesh() const { return m_mesh; }

		SkeletalMeshComponent& mesh(SkeletalMesh* mesh)
		{
			m_mesh = mesh;
			return update_bounding_box();
		}
	};
}// namespace Trinex
