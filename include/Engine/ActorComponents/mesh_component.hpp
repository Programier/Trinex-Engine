#pragma once
#include <Engine/ActorComponents/primitive_component.hpp>

namespace Trinex
{
	struct RHISemantic;
	struct MeshVertexAttribute;
	class MaterialInterface;

	class ENGINE_EXPORT MeshComponent : public PrimitiveComponent
	{
		trinex_class(MeshComponent, PrimitiveComponent);

	private:
		Vector<MaterialInterface*> m_material_overrides;

	public:
		virtual MaterialInterface* material(usize index) const;
		virtual MeshComponent& material(MaterialInterface*, usize index);

		inline usize overrided_material_count() const { return m_material_overrides.size(); }
		inline MaterialInterface* overrided_material(usize index) const { return m_material_overrides[index]; }
	};
}// namespace Trinex
