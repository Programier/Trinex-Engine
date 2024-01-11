#pragma once
#include <Graphics/visual_material.hpp>

namespace Engine::MaterialNodes
{
    enum class Type : EnumerateType
    {
        GBufferRoot = 1,

        Sin = 10,
        Cos = 11,
        Max = 12,
    };
}
