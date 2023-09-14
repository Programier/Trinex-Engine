#pragma once
#include <Core/engine_types.hpp>

namespace Engine::Mouse
{
    enum ButtonStatus : EnumerateType
    {
        Released = 0,
        JustReleased,
        JustPressed,
        Pressed,
    };

    enum Button : EnumerateType
    {
        Left,
        Middle,
        Right,
        X1,
        X2,
        __COUNT__
    };

    enum Direction : EnumerateType
    {
        None = 0,
        Normal,
        Flipped,
    };
}
