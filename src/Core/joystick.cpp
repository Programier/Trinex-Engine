#include <Core/engine_loading_controllers.hpp>
#include <Core/joystick.hpp>
#include <Core/logger.hpp>


namespace Engine
{
	TreeMap<Identifier, Joystick*> Joystick::m_joysticks;

	Joystick::Joystick(Identifier ID) : m_ID(ID)
	{
		info_log("Joystick", "Created new joystick instance with ID %zu", ID);
		m_joysticks[ID] = this;

		//   SDL_Joystick* sdl_joystick = SDL_JoystickOpen(ID);
		///m_SDL_joystick            = sdl_joystick;
	}

	Identifier Joystick::id() const
	{
		return m_ID;
	}

	TreeMap<Identifier, Joystick*> Joystick::joysticks()
	{
		return m_joysticks;
	}

	Joystick* Joystick::find_joystick(Identifier ID)
	{
		auto it = m_joysticks.find(ID);
		if (it != m_joysticks.end())
		{
			return it->second;
		}

		return nullptr;
	}

	Joystick::~Joystick()
	{
		info_log("Joystick", "Removed joystick instance with ID %zu", m_ID);
		m_joysticks.erase(id());

		//SDL_Joystick* joystick = reinterpret_cast<SDL_Joystick*>(m_SDL_joystick);
		//SDL_JoystickClose(joystick);
	}

	static void on_destroy()
	{
		while (!Joystick::joysticks().empty())
		{
			Joystick* joystick = Joystick::joysticks().begin()->second;
			delete joystick;
		}
	}

	static DestroyController on_destroy_controller(on_destroy);
}// namespace Engine
