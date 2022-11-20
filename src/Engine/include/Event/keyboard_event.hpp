#pragma once

#include <Core/keyboard.hpp>
#include <list>

namespace Engine
{
    STRUCT KeyboardEvent
    {
        static ENGINE_EXPORT const Key just_pressed();
        static ENGINE_EXPORT unsigned int last_symbol(bool reset = true);
        static ENGINE_EXPORT const Key last_pressed();
        static ENGINE_EXPORT const Key just_released();
        static ENGINE_EXPORT KeyStatus get_key_status(const Key& key);
        static ENGINE_EXPORT bool pressed(const Key& key);

        template <typename... Args>
        static bool pressed(const Key& key, const Key& second_key, const Args&... args)
        {
            return pressed(key) && pressed(second_key, args...);
        }

        static ENGINE_EXPORT const std::list<Key>& just_evented_keys();
        static ENGINE_EXPORT void push_event(Key key, KeyStatus status);
    };
}
