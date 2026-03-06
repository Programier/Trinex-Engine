#pragma once
#include <Engine/ActorComponents/primitive_component.hpp>

namespace Engine
{
	struct RHIVertexSemantic;
	struct MeshVertexAttribute;
	class MaterialInterface;

	class ENGINE_EXPORT MeshComponent : public PrimitiveComponent
	{
		trinex_class(MeshComponent, PrimitiveComponent);

	private:
		Vector<MaterialInterface*> m_material_overrides;

	public:
		MaterialInterface* material(usize index) const override;
		MeshComponent& material(MaterialInterface*, usize index) override;

		virtual usize lods_count() const                                                               = 0;
		virtual usize surfaces_count(usize lod = 0) const                                              = 0;
		virtual const MeshSurface* surface(usize index, usize lod = 0) const                           = 0;
		virtual const MeshVertexAttribute* vertex_attribute(RHIVertexSemantic semantic, usize lod = 0) = 0;
		virtual VertexBufferBase* vertex_buffer(u8 stream, usize lod = 0)                              = 0;
		virtual IndexBuffer* index_buffer(usize lod = 0)                                               = 0;
		MeshComponent& render(PrimitiveRenderingContext* context) override;

		inline usize overrided_material_count() const { return m_material_overrides.size(); }
		inline MaterialInterface* overrided_material(usize index) const { return m_material_overrides[index]; }
	};
}// namespace Engine
