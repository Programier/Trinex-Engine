#pragma once
#include <Core/keyboard.hpp>


#include <Event/event.hpp>

namespace Engine
{
    struct Keys {
        Vector<KeyStatus> _M_keys;
        Key _M_last_key = Key::KEY_UNKNOWN;
        Key _M_last_released = Key::KEY_UNKNOWN;
        uint_t _M_last_symbol = 0;
        Key _M_last_mouse_key = Key::KEY_UNKNOWN;
        Key _M_last_mouse_released = Key::KEY_UNKNOWN;

        List<Key> _M_last_evented_keys;

        Keys()
        {
            _M_keys.resize(key_count());
        }
    };

    Keys& get_keys_address();
    const Event& get_event();
    void on_leave();
}// namespace Engine