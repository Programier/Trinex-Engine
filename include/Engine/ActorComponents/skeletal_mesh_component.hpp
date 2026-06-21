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
		using MeshComponent::material;

		MaterialInterface* material(usize index) const override;

		inline SkeletalMesh* mesh() const { return m_mesh; }

		SkeletalMeshComponent& mesh(SkeletalMesh* mesh)
		{
			m_mesh = mesh;
			return *this;
		}
	};
}// namespace Trinex
