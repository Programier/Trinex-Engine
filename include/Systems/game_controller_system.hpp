#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/singletone.hpp>
#include <Systems/system.hpp>


namespace Engine
{
	class GameController;
	struct Event;

	class GameControllerSystem : public Singletone<GameControllerSystem, System>
	{
		trinex_class(GameControllerSystem, System);


	private:
		Map<Identifier, GameController*> m_controllers;
		Identifier m_listener_ids[3];

		void on_controller_added(const Event& event);
		void on_controller_removed(const Event& event);

		GameControllerSystem();

		void on_axis_motion(const Event& event);

	protected:
		virtual GameControllerSystem& create() override;

	public:
		virtual GameControllerSystem& update(float dt) override;
		virtual GameControllerSystem& shutdown() override;
		GameController* controller(Identifier id) const;

		friend class Singletone<GameControllerSystem, System>;
	};
}// namespace Engine
