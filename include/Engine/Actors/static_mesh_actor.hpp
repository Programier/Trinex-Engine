#pragma once

#include <Engine/Actors/actor.hpp>

namespace Engine
{
	class ENGINE_EXPORT StaticMeshActor : public Actor
	{
		trinex_class(StaticMeshActor, Actor);

	private:
		class StaticMeshComponent* m_mesh_component = nullptr;

	public:
		StaticMeshComponent* mesh_component() const;

		StaticMeshActor();
		~StaticMeshActor();
	};
}// namespace Engine
