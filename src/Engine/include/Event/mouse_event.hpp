#pragma once

#include <Core/keyboard.hpp>
#include <Core/engine_types.hpp>
#include <list>

namespace Engine
{
    STRUCT MouseEvent
    {
        static ENGINE_EXPORT const Point2D& position();
        static ENGINE_EXPORT void position(const Point2D& position);
        static ENGINE_EXPORT const Offset2D& offset();
        static ENGINE_EXPORT const Offset2D& scroll_offset();

        static ENGINE_EXPORT const Key just_pressed();
        static ENGINE_EXPORT const Key last_pressed();
        static ENGINE_EXPORT const Key just_released();

        static ENGINE_EXPORT KeyStatus get_key_status(const Key& key);
        static ENGINE_EXPORT bool pressed(const Key& key);
        static ENGINE_EXPORT const std::list<Key>& just_evented_keys();

        static ENGINE_EXPORT bool relative_mode();
        static ENGINE_EXPORT void relative_mode(bool value);

    };
}
