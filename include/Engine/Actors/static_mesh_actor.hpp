#pragma once

#include <Engine/Actors/actor.hpp>

namespace Trinex
{
	class trinex_api StaticMeshActor : public Actor
	{
		trinex_class(StaticMeshActor, Actor);

	private:
		class StaticMeshComponent* m_mesh_component = nullptr;

	public:
		StaticMeshComponent* mesh_component() const;

		StaticMeshActor();
		~StaticMeshActor();
	};
}// namespace Trinex
