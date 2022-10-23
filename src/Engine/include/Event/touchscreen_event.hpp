#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>

namespace Engine
{
    STRUCT Finger {
        bool on_screen = false;
        Point2D position = {-1.f, -1.f};
        Point2D offset = {0.f, 0.f};
        float pressure = 0;
    };

    STRUCT TouchScreenEvent {
        static ENGINE_EXPORT unsigned int fingers_count();
        static ENGINE_EXPORT unsigned int prev_fingers_count();
        static ENGINE_EXPORT const Finger& get_finger(unsigned int index);
    };
}// namespace Engine
