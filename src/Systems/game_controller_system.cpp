#include <Core/event.hpp>
#include <Core/game_controller.hpp>
#include <Systems/game_controller_system.hpp>

namespace Trinex
{
	GameControllerSystem* GameControllerSystem::s_instance = nullptr;

	GameControllerSystem::GameControllerSystem() = default;

	GameController* GameControllerSystem::controller(Identifier id) const
	{
		if (auto found = m_controllers.find(id); found != m_controllers.end())
		{
			return found->second;
		}

		return nullptr;
	}

	void GameControllerSystem::on_event(const Event& event)
	{
		switch (event.type)
		{
			case EventType::ControllerDeviceAdded:
				if (m_controllers.find(event.gamepad.id) == m_controllers.end())
				{
					m_controllers[event.gamepad.id] = trx_new GameController(event.gamepad.id);
				}
				break;

			case EventType::ControllerAxisMotion:
				if (GameController* controller = this->controller(event.gamepad.id))
				{
					controller->axis_motion_listener(event);
				}
				break;

			case EventType::ControllerDeviceRemoved:
				if (auto found = m_controllers.find(event.gamepad.id); found != m_controllers.end())
				{
					found->second->controller_removed_listener();
					trx_delete found->second;
					m_controllers.erase(found);
				}
				break;

			default: break;
		}
	}

	GameControllerSystem::~GameControllerSystem()
	{
		for (auto& [id, controller] : m_controllers)
		{
			trx_delete controller;
		}
	}
}// namespace Trinex
