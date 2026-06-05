#pragma once

#include <Core/etl/singletone.hpp>
#include <Core/keyboard.hpp>

namespace Trinex
{
	class ENGINE_EXPORT KeyboardSystem : public Singletone<KeyboardSystem, EmptySingletoneParent>
	{
	public:
		static KeyboardSystem* s_instance;

	private:
		friend class Singletone<KeyboardSystem, EmptySingletoneParent>;

		Keyboard::Status m_states[Keyboard::__COUNT__] = {};

		KeyboardSystem();

	public:
		KeyboardSystem& begin_frame();
		bool is_pressed(Keyboard::Key key) const;
		bool is_released(Keyboard::Key key) const;
		bool is_just_pressed(Keyboard::Key key) const;
		bool is_just_released(Keyboard::Key key) const;
		void on_event(const struct Event& event);
	};
}// namespace Trinex
