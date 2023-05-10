#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>

namespace Engine
{
    struct ENGINE_EXPORT Finger {
        bool on_screen = false;
        Point2D position = {-1.f, -1.f};
        Point2D offset = {0.f, 0.f};
        float pressure = 0;
    };

    struct ENGINE_EXPORT TouchScreenEvent {
        static ENGINE_EXPORT uint_t fingers_count();
        static ENGINE_EXPORT uint_t prev_fingers_count();
        static ENGINE_EXPORT const Finger& get_finger(uint_t index);
    };
}// namespace Engine
