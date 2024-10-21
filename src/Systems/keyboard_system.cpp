#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Event/event.hpp>
#include <Event/event_data.hpp>
#include <Systems/event_system.hpp>
#include <Systems/keyboard_system.hpp>

namespace Engine
{
	void KeyboardSystem::on_key_pressed(const Event& event)
	{
		const KeyEvent& key_event = event.get<const KeyEvent&>();
		m_last_pressed_keys.push_back(key_event.key);
		set(key_event.key, Keyboard::JustPressed);
	}

	void KeyboardSystem::on_key_released(const Event& event)
	{
		const KeyEvent& key_event = event.get<const KeyEvent&>();

		m_last_released_keys.push_back(key_event.key);
		set(key_event.key, Keyboard::JustReleased);
	}

	KeyboardSystem& KeyboardSystem::set(Keyboard::Key key, Keyboard::Status status)
	{
		m_key_status[static_cast<EnumerateType>(key)] = status;
		return *this;
	}

	KeyboardSystem& KeyboardSystem::create()
	{
		Super::create();

		std::fill(m_key_status, m_key_status + Keyboard::__COUNT__, Keyboard::Released);
		EventSystem* event_system = System::new_system<EventSystem>();
		event_system->register_subsystem(this);

		m_key_press_id = event_system->add_listener(EventType::KeyDown,
		                                            std::bind(&KeyboardSystem::on_key_pressed, this, std::placeholders::_1));

		m_key_release_id = event_system->add_listener(EventType::KeyUp,
		                                              std::bind(&KeyboardSystem::on_key_released, this, std::placeholders::_1));
		return *this;
	}

	KeyboardSystem& KeyboardSystem::wait()
	{
		Super::wait();
		return *this;
	}


	void KeyboardSystem::process_last_keys(Vector<Keyboard::Key>& vector, Keyboard::Status status)
	{
		for (Keyboard::Key key : vector)
		{
			m_key_status[static_cast<EnumerateType>(key)] = status;
		}

		if (!vector.empty())
		{
			vector.clear();
		}
	}

	KeyboardSystem& KeyboardSystem::update(float dt)
	{
		Super::update(dt);
		process_last_keys(m_last_pressed_keys, Keyboard::Pressed);
		process_last_keys(m_last_released_keys, Keyboard::Released);
		return *this;
	}

	KeyboardSystem& KeyboardSystem::shutdown()
	{
		Super::shutdown();

		EventSystem* event_system = EventSystem::instance();
		event_system->remove_listener(m_key_release_id);
		event_system->remove_listener(m_key_press_id);

		return *this;
	}

	Keyboard::Status KeyboardSystem::status_of(Keyboard::Key key) const
	{
		return m_key_status[static_cast<EnumerateType>(key)];
	}

	bool KeyboardSystem::is_pressed(Keyboard::Key key) const
	{
		Keyboard::Status status = status_of(key);
		return status == Keyboard::Pressed || status == Keyboard::JustPressed;
	}

	bool KeyboardSystem::is_released(Keyboard::Key key) const
	{
		return !is_pressed(key);
	}

	bool KeyboardSystem::is_just_pressed(Keyboard::Key key) const
	{
		return status_of(key) == Keyboard::JustPressed;
	}

	bool KeyboardSystem::is_just_released(Keyboard::Key key) const
	{
		return status_of(key) == Keyboard::JustReleased;
	}

	implement_engine_class_default_init(KeyboardSystem, 0);
}// namespace Engine
