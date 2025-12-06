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
		MaterialInterface* material(size_t index) const override;
		MeshComponent& material(MaterialInterface*, size_t index) override;

		virtual size_t lods_count() const                                                               = 0;
		virtual size_t surfaces_count(size_t lod = 0) const                                             = 0;
		virtual const MeshSurface* surface(size_t index, size_t lod = 0) const                          = 0;
		virtual const MeshVertexAttribute* vertex_attribute(RHIVertexSemantic semantic, size_t lod = 0) = 0;
		virtual VertexBufferBase* vertex_buffer(byte stream, size_t lod = 0)                            = 0;
		virtual IndexBuffer* index_buffer(size_t lod = 0)                                               = 0;
		MeshComponent& render(PrimitiveRenderingContext* context) override;

		inline size_t overrided_material_count() const { return m_material_overrides.size(); }
		inline MaterialInterface* overrided_material(size_t index) const { return m_material_overrides[index]; }
	};
}// namespace Engine
