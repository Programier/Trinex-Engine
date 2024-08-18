#pragma once
#include <Core/etl/singletone.hpp>
#include <Event/listener_id.hpp>
#include <Systems/system.hpp>


namespace Engine
{
	class GameController;

	class GameControllerSystem : public Singletone<GameControllerSystem, System>
	{
		declare_class(GameControllerSystem, System);


	private:
		Map<Identifier, GameController*> m_controllers;
		Vector<EventSystemListenerID> m_callbacks_id;

		void on_controller_added(const Event& event);
		void on_controller_removed(const Event& event);
		void on_axis_motion(const Event& event);


		GameControllerSystem();

	public:
		virtual GameControllerSystem& create() override;
		virtual GameControllerSystem& wait() override;
		virtual GameControllerSystem& update(float dt) override;
		virtual GameControllerSystem& shutdown() override;
		GameController* controller(Identifier id) const;

		friend class Singletone<GameControllerSystem, System>;
	};
}// namespace Engine
