#include <Event/keyboard_event.hpp>
#include <Event/mouse_event.hpp>
#include <SDL_events.h>
#include <Window/window.hpp>
#include <private_event.hpp>

namespace Engine
{
    static Keys& keys = Engine::get_keys_address();
    static Point2D _M_mouse_position = {-1.f, -1.f};
    static Point2D _M_mouse_offset = {0.f, 0.f};
    static Point2D _M_scroll_offset = {0.f, 0.f};

    void mouse_process_event(SDL_MouseMotionEvent& event)
    {
        _M_mouse_position = {event.x, event.y};
        _M_mouse_offset = {event.xrel, event.yrel};
    }

    void mouse_process_event(SDL_MouseButtonEvent& event)
    {
        Key key = to_key(event.button);
        if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            keys._M_keys[key] = KeyStatus::JUST_PRESSED;
            keys._M_last_mouse_key = key;
        }
        else
        {
            keys._M_keys[key] = KeyStatus::JUST_RELEASED;
            keys._M_last_mouse_released = key;
        }

        keys._M_last_evented_keys.push_back(key);
    }


    void mouse_process_event(SDL_MouseWheelEvent& event)
    {
        _M_scroll_offset = {event.preciseX, event.preciseY};
    }

    void clear_mouse_event()
    {
        _M_scroll_offset = {0, 0};
        _M_mouse_offset = {0, 0};
    }


    ENGINE_EXPORT const Point2D& MouseEvent::position()
    {
        return _M_mouse_position;
    }

    ENGINE_EXPORT void MouseEvent::position(const Point2D& position)
    {
        SDL_WarpMouseInWindow(cast(SDL_Window*, Window::SDL()), cast(int, position.x), cast(int, position.y));
    }

    ENGINE_EXPORT const Offset2D& MouseEvent::offset()
    {
        return _M_mouse_offset;
    }

    ENGINE_EXPORT const Offset2D& MouseEvent::scroll_offset()
    {
        return _M_scroll_offset;
    }

    ENGINE_EXPORT const Key MouseEvent::just_pressed()
    {
        return keys._M_keys[keys._M_last_mouse_key] == KeyStatus::JUST_PRESSED ? to_key(keys._M_last_mouse_key - 1)
                                                                               : KEY_UNKNOWN;
    }

    ENGINE_EXPORT const Key MouseEvent::last_pressed()
    {
        return keys._M_last_mouse_key;
    }

    ENGINE_EXPORT const Key MouseEvent::just_released()
    {
        return keys._M_keys[keys._M_last_mouse_released] == KeyStatus::JUST_RELEASED
                       ? to_key(keys._M_last_mouse_released - 1)
                       : KEY_UNKNOWN;
    }

    ENGINE_EXPORT KeyStatus MouseEvent::get_key_status(const Key& key)
    {
        return KeyboardEvent::get_key_status(key);
    }

    ENGINE_EXPORT bool MouseEvent::pressed(const Key& key)
    {
        return KeyboardEvent::pressed(key);
    }

    ENGINE_EXPORT const std::list<Key>& MouseEvent::just_evented_keys()
    {
        return KeyboardEvent::just_evented_keys();
    }


    void on_leave()
    {
        _M_mouse_position = {-1.f, -1.f};
    }

}// namespace Engine