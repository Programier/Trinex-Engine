#pragma once
#include <Graphics/visual_material.hpp>

namespace Engine::MaterialNodes
{
    enum class Type : EnumerateType
    {
        GBufferRoot,


        // Constants group
        Bool,
        Int,
        UInt,
        Float,
        BVec2,
        BVec3,
        BVec4,
        IVec2,
        IVec3,
        IVec4,
        UVec2,
        UVec3,
        UVec4,
        Vec2,
        Vec3,
        Vec4,
        Color3,
        Color4,

        // Math group
        Sin,
        Cos,
        Tan,
        ASin,
        ACos,
        ATan,

        Max,
        Min

    };
}
