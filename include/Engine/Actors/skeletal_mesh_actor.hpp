#pragma once

#include <Engine/Actors/actor.hpp>

namespace Trinex
{
	class trinex_api SkeletalMeshActor : public Actor
	{
		trinex_class(SkeletalMeshActor, Actor);

	private:
		class SkeletalMeshComponent* m_mesh_component = nullptr;

	public:
		SkeletalMeshComponent* mesh_component() const;

		SkeletalMeshActor();
		~SkeletalMeshActor();
	};
} // namespace Trinex
