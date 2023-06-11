#include <Event/keyboard_event.hpp>
#include <SDL_events.h>

#include <private_event.hpp>


namespace Engine
{
    Keys keys;

    Keys& get_keys_address()
    {
        return keys;
    }


    ENGINE_EXPORT Key KeyboardEvent::just_pressed()
    {
        return keys._M_keys[static_cast<EnumerateType>(keys._M_last_key)] == KeyStatus::JustPressed ? keys._M_last_key
                                                                                                    : Key::Unknown;
    }

    ENGINE_EXPORT bool KeyboardEvent::just_pressed(Key key)
    {
        return just_pressed() == key;
    }

    ENGINE_EXPORT bool KeyboardEvent::last_pressed(Key key)
    {
        return last_pressed() == key;
    }

    ENGINE_EXPORT bool KeyboardEvent::just_released(Key key)
    {
        return just_released() == key;
    }

    ENGINE_EXPORT const List<Key>& KeyboardEvent::just_evented_keys()
    {
        return keys._M_last_evented_keys;
    }

    ENGINE_EXPORT uint_t KeyboardEvent::last_symbol(bool reset)
    {
        auto tmp            = keys._M_last_symbol;
        keys._M_last_symbol = 0;
        return tmp;
    }

    ENGINE_EXPORT Key KeyboardEvent::last_pressed()
    {
        return keys._M_last_key;
    }

    ENGINE_EXPORT Key KeyboardEvent::just_released()
    {
        return keys._M_keys[static_cast<EnumerateType>(keys._M_last_released)] == KeyStatus::JustReleased
                       ? keys._M_last_released
                       : Key::Unknown;
    }

    ENGINE_EXPORT KeyStatus KeyboardEvent::get_key_status(const Key& key)
    {
        return keys._M_keys[static_cast<unsigned int>(key)];
    }

    ENGINE_EXPORT bool KeyboardEvent::pressed(const Key& key)
    {
        return keys._M_keys[static_cast<unsigned int>(key)] != KeyStatus::Released &&
               keys._M_keys[static_cast<unsigned int>(key)] != KeyStatus::JustReleased;
    }

    ENGINE_EXPORT void KeyboardEvent::push_event(Key key, KeyStatus status)
    {
        static SDL_Event event;
        if (status == KeyStatus::Released || status == KeyStatus::JustReleased)
            event.type = SDL_KEYUP;
        else
            event.type = SDL_KEYDOWN;

        event.key.repeat          = status == KeyStatus::Repeat;
        event.key.type            = event.type;
        event.key.keysym.scancode = (SDL_Scancode) Engine::to_SDL_scancode(key);
        SDL_PushEvent(&event);
    }


    void process_keyboard_event(SDL_KeyboardEvent& event)
    {
        Key key           = to_key(event.keysym.scancode);
        KeyStatus& status = keys._M_keys[static_cast<EnumerateType>(key)];

        if (event.repeat)
        {
            status = KeyStatus::Repeat;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            status           = KeyStatus::JustPressed;
            keys._M_last_key = key;
            keys._M_last_evented_keys.push_back(key);
            keys._M_last_symbol = SDL_GetKeyFromScancode(event.keysym.scancode);
        }
        else
        {
            status                = KeyStatus::JustReleased;
            keys._M_last_released = key;
            keys._M_last_evented_keys.push_back(key);
        }
    }

    void clear_keyboard_events()
    {
        for (auto key : keys._M_last_evented_keys)
        {
            if (keys._M_keys[static_cast<EnumerateType>(key)] == KeyStatus::JustPressed)
                keys._M_keys[static_cast<EnumerateType>(key)] = KeyStatus::Pressed;
            if (keys._M_keys[static_cast<EnumerateType>(key)] == KeyStatus::JustReleased)
                keys._M_keys[static_cast<EnumerateType>(key)] = KeyStatus::Released;
        }

        keys._M_last_evented_keys.clear();
    }


}// namespace Engine
