#pragma once

#include <Engine/Actors/actor.hpp>

namespace Engine
{
	class ENGINE_EXPORT SkeletalMeshActor : public Actor
	{
		trinex_class(SkeletalMeshActor, Actor);

	private:
		class SkeletalMeshComponent* m_mesh_component = nullptr;

	public:
		SkeletalMeshComponent* mesh_component() const;

		SkeletalMeshActor();
		~SkeletalMeshActor();
	};
}// namespace Engine
