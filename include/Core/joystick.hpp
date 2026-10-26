#pragma once

#include <Core/engine_types.hpp>
#include <Core/etl/map.hpp>

namespace Trinex
{
	class trinex_api Joystick
	{
	private:
		static TreeMap<Identifier, Joystick*> m_joysticks;
		Identifier m_ID;
		Joystick(Identifier ID);

	public:
		Identifier id() const;
		static TreeMap<Identifier, Joystick*> joysticks();
		static Joystick* find_joystick(Identifier ID);

		~Joystick();
		friend struct JoystickEvent;
	};
} // namespace Trinex
