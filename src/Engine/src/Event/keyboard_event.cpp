#include <Event/keyboard_event.hpp>
#include <SDL_events.h>
#include <list>
#include <private_event.hpp>
#include <vector>

namespace Engine
{
    Keys keys;

    Keys& get_keys_address()
    {
        return keys;
    }


    ENGINE_EXPORT const Key KeyboardEvent::just_pressed()
    {
        return keys._M_keys[keys._M_last_key] == KeyStatus::JUST_PRESSED ? keys._M_last_key : KEY_UNKNOWN;
    }

    ENGINE_EXPORT const std::list<Key>& KeyboardEvent::just_evented_keys()
    {
        return keys._M_last_evented_keys;
    }

    ENGINE_EXPORT unsigned int KeyboardEvent::last_symbol(bool reset)
    {
        auto tmp = keys._M_last_symbol;
        keys._M_last_symbol = 0;
        return tmp;
    }

    ENGINE_EXPORT const Key KeyboardEvent::last_pressed()
    {
        return keys._M_last_key;
    }

    ENGINE_EXPORT const Key KeyboardEvent::just_released()
    {
        return keys._M_keys[keys._M_last_released] == KeyStatus::JUST_RELEASED ? keys._M_last_released : KEY_UNKNOWN;
    }

    ENGINE_EXPORT KeyStatus KeyboardEvent::get_key_status(const Key& key)
    {
        return keys._M_keys[static_cast<unsigned int>(key)];
    }

    ENGINE_EXPORT bool KeyboardEvent::pressed(const Key& key)
    {
        return keys._M_keys[static_cast<unsigned int>(key)] != KeyStatus::RELEASED &&
               keys._M_keys[static_cast<unsigned int>(key)] != KeyStatus::JUST_RELEASED;
    }

    ENGINE_EXPORT void KeyboardEvent::push_event(Key key, KeyStatus status)
    {
        static SDL_Event event;
        if (status == KeyStatus::RELEASED || status == KeyStatus::JUST_RELEASED)
            event.type = SDL_KEYUP;
        else
            event.type = SDL_KEYDOWN;

        event.key.repeat = status == KeyStatus::REPEAT;
        event.key.type = event.type;
        event.key.keysym.scancode = (SDL_Scancode) Engine::to_SDL_scancode(key);
        SDL_PushEvent(&event);
    }


    void process_keyboard_event(SDL_KeyboardEvent& event)
    {
        Key key = to_key(event.keysym.scancode);
        KeyStatus& status = keys._M_keys[key];

        if (event.repeat)
        {
            status = KeyStatus::REPEAT;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            status = KeyStatus::JUST_PRESSED;
            keys._M_last_key = key;
            keys._M_last_evented_keys.push_back(key);
        }
        else
        {
            status = KeyStatus::JUST_RELEASED;
            keys._M_last_released = key;
            keys._M_last_evented_keys.push_back(key);
        }
    }

    void clear_keyboard_events()
    {
        for (auto key : keys._M_last_evented_keys)
        {
            if (keys._M_keys[key] == KeyStatus::JUST_PRESSED)
                keys._M_keys[key] = KeyStatus::PRESSED;
            if (keys._M_keys[key] == KeyStatus::JUST_RELEASED)
                keys._M_keys[key] = KeyStatus::RELEASED;
        }

        keys._M_last_evented_keys.clear();
    }


}// namespace Engine
