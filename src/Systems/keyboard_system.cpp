#include <Core/event.hpp>
#include <Systems/keyboard_system.hpp>

namespace Trinex
{
	KeyboardSystem* KeyboardSystem::s_instance = nullptr;

	KeyboardSystem::KeyboardSystem()
	{
		for (auto& state : m_states)
		{
			state = Keyboard::Released;
		}
	}

	KeyboardSystem& KeyboardSystem::begin_frame()
	{
		for (auto& state : m_states)
		{
			if (state == Keyboard::JustPressed)
				state = Keyboard::Pressed;
			else if (state == Keyboard::JustReleased)
				state = Keyboard::Released;
		}

		return *this;
	}

	bool KeyboardSystem::is_pressed(Keyboard::Key key) const
	{
		return m_states[key] == Keyboard::Pressed || m_states[key] == Keyboard::JustPressed;
	}

	bool KeyboardSystem::is_released(Keyboard::Key key) const
	{
		return !is_pressed(key);
	}

	bool KeyboardSystem::is_just_pressed(Keyboard::Key key) const
	{
		return m_states[key] == Keyboard::JustPressed;
	}

	bool KeyboardSystem::is_just_released(Keyboard::Key key) const
	{
		return m_states[key] == Keyboard::JustReleased;
	}

	void KeyboardSystem::on_event(const Event& event)
	{
		if (event.type != EventType::KeyDown && event.type != EventType::KeyUp)
			return;

		const Keyboard::Key key = event.keyboard.key;
		if (key < 0 || key >= Keyboard::__COUNT__)
			return;

		m_states[key] = event.type == EventType::KeyDown ? Keyboard::JustPressed : Keyboard::JustReleased;
	}
}// namespace Trinex
