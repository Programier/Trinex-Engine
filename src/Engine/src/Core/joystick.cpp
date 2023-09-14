#include <Core/engine_loading_controllers.hpp>
#include <Core/joystick.hpp>
#include <Core/logger.hpp>


namespace Engine
{
    TreeMap<Identifier, Joystick*> Joystick::_M_joysticks;

    Joystick::Joystick(Identifier ID) : _M_ID(ID)
    {
        info_log("Joystick", "Created new joystick instance with ID %zu", ID);
        _M_joysticks[ID] = this;

        //   SDL_Joystick* sdl_joystick = SDL_JoystickOpen(ID);
        ///_M_SDL_joystick            = sdl_joystick;
    }

    Identifier Joystick::id() const
    {
        return _M_ID;
    }

    TreeMap<Identifier, Joystick*> Joystick::joysticks()
    {
        return _M_joysticks;
    }

    Joystick* Joystick::find_joystick(Identifier ID)
    {
        auto it = _M_joysticks.find(ID);
        if (it != _M_joysticks.end())
        {
            return it->second;
        }

        return nullptr;
    }

    Joystick::~Joystick()
    {
        info_log("Joystick", "Removed joystick instance with ID %zu", _M_ID);
        _M_joysticks.erase(id());

        //SDL_Joystick* joystick = reinterpret_cast<SDL_Joystick*>(_M_SDL_joystick);
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
