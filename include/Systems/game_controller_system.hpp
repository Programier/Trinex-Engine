#pragma once

#include <Core/etl/map.hpp>
#include <Core/etl/singletone.hpp>

namespace Trinex
{
	struct Event;
	class GameController;

	class ENGINE_EXPORT GameControllerSystem : public Singletone<GameControllerSystem, EmptySingletoneParent>
	{
	public:
		static GameControllerSystem* s_instance;

	private:
		friend class Singletone<GameControllerSystem, EmptySingletoneParent>;

		Map<Identifier, GameController*> m_controllers;
		GameControllerSystem();

	public:
		GameController* controller(Identifier id) const;
		void on_event(const Event& event);
		~GameControllerSystem();
	};
}// namespace Trinex
