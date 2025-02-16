#include <Core/game_controller.hpp>
#include <Core/reflection/class.hpp>
#include <Systems/event_system.hpp>
#include <Systems/game_controller_system.hpp>

namespace Engine
{
	implement_engine_class_default_init(GameControllerSystem, 0);

#define new_id(x) m_callbacks_id.push_back(x)

	void GameControllerSystem::on_controller_added(const Event& event)
	{
		//const ControllerDeviceAddedEvent& e = event.get<const ControllerDeviceAddedEvent&>();
		//m_controllers[e.id]                = new GameController(e.id);
	}

	void GameControllerSystem::on_controller_removed(const Event& event)
	{
		// const ControllerDeviceRemovedEvent& e = event.get<const ControllerDeviceRemovedEvent&>();
		// delete m_controllers[e.id];
		// m_controllers.erase(e.id);
	}

	void GameControllerSystem::on_axis_motion(const Event& event)
	{
		auto& motion                                            = event.gamepad.axis_motion;
		m_controllers.at(motion.id)->m_axis_values[motion.axis] = motion.value;
	}


	GameControllerSystem::GameControllerSystem()
	{}

	GameControllerSystem& GameControllerSystem::create()
	{
		Super::create();

		EventSystem* event_system = System::system_of<EventSystem>();
		event_system->register_subsystem(this);

		new_id(event_system->add_listener(EventType::ControllerDeviceAdded,
		                                  std::bind(&GameControllerSystem::on_controller_added, this, std::placeholders::_1)));

		new_id(event_system->add_listener(EventType::ControllerDeviceRemoved,
		                                  std::bind(&GameControllerSystem::on_controller_removed, this, std::placeholders::_1)));

		new_id(event_system->add_listener(EventType::ControllerAxisMotion,
		                                  std::bind(&GameControllerSystem::on_axis_motion, this, std::placeholders::_1)));

		return *this;
	}

	GameControllerSystem& GameControllerSystem::wait()
	{
		Super::wait();
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

}// namespace Engine
