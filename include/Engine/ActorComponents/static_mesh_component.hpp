#pragma once
#include <Core/constants.hpp>
#include <Core/pointer.hpp>
#include <Engine/ActorComponents/mesh_component.hpp>
#include <Graphics/gpu_buffers.hpp>


namespace Engine
{
	class ENGINE_EXPORT StaticMeshComponent : public MeshComponent
	{
		trinex_declare_class(StaticMeshComponent, MeshComponent);

	public:
		class StaticMesh* mesh = nullptr;

		StaticMeshComponent& update(float dt) override;
		StaticMeshComponent& render(class SceneRenderer*) override;
		StaticMeshComponent& update_bounding_box() override;
		MaterialInterface* material(size_t index) const;
	};
}// namespace Engine
