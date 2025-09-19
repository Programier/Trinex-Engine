#include <Core/game_controller.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <ScriptEngine/registrar.hpp>
#include <Systems/event_system.hpp>
#include <Systems/game_controller_system.hpp>

namespace Engine
{
#define new_listener(type, func)                                                                                                 \
	m_listener_ids[listener_index] = event_system->add_listener(                                                                 \
	        EventType::Controller##type, std::bind(&GameControllerSystem::func, this, std::placeholders::_1))

	void GameControllerSystem::on_controller_added(const Event& event)
	{
		m_controllers[event.gamepad.id] = trx_new GameController(event.gamepad.id);
	}

	void GameControllerSystem::on_controller_removed(const Event& event)
	{
		auto controller = m_controllers.at(event.gamepad.id);

		if (controller)
		{
			controller->controller_removed_listener();

			logic_thread()->call([controller]() {
				GameControllerSystem* system = GameControllerSystem::instance();
				if (system)
				{
					system->m_controllers.erase(controller->id());
					trx_delete controller;
				}
			});
		}
	}

	void GameControllerSystem::on_axis_motion(const Event& event)
	{
		if (auto device = controller(event.gamepad.id))
		{
			device->axis_motion_listener(event);
		}
	}

	GameControllerSystem::GameControllerSystem() {}

	GameControllerSystem& GameControllerSystem::create()
	{
		Super::create();

		EventSystem* event_system = System::system_of<EventSystem>();
		event_system->register_subsystem(this);


		int listener_index = 0;
		new_listener(DeviceAdded, on_controller_added);
		new_listener(DeviceRemoved, on_controller_removed);
		new_listener(AxisMotion, on_axis_motion);

		return *this;
	}

	GameControllerSystem& GameControllerSystem::update(float dt)
	{
		Super::update(dt);
		return *this;
	}

	GameControllerSystem& GameControllerSystem::shutdown()
	{
		Super::shutdown();
		return *this;
	}

	GameController* GameControllerSystem::controller(Identifier id) const
	{
		auto it = m_controllers.find(id);
		if (it != m_controllers.end())
		{
			return it->second;
		}

		return nullptr;
	}

	trinex_implement_engine_class(GameControllerSystem, Refl::Class::IsScriptable) {}

}// namespace Engine
