#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    struct ENGINE_EXPORT WorldEnvironment {
        Color3 ambient_color = Color3(0.1, 0.1, 0.1);
    };
}// namespace Engine
