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
		u32 m_transform          = 0;
		u32 m_geometry           = 0;
		u32 m_primitive          = 0;

	public:
		using MeshComponent::material;
		MaterialInterface* material(usize index) const override;

		StaticMeshComponent& start_play() override;
		StaticMeshComponent& stop_play() override;
		StaticMeshComponent& on_transform_changed() override;

		inline StaticMesh* mesh() const { return m_mesh; }

		inline StaticMeshComponent& mesh(StaticMesh* mesh)
		{
			m_mesh = mesh;
			return *this;
		}
	};
}// namespace Trinex
