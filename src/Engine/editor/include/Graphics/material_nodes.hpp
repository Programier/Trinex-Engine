#pragma once
#include <Core/engine_types.hpp>

namespace Engine::MaterialNodes
{
    enum class Type : EnumerateType
    {
        VertexRoot,
        FragmentRoot,


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

        SinH,
        CosH,
        TanH,
        ASinH,
        ACosH,
        ATanH,

        Pow,
        Exp,
        Log,
        Exp2,
        Log2,

        Sqrt,
        InverseSqrt,
        Abs,
        Sign,
        Floor,
        Ceil,
        Fract,
        Mod,
        Min,
        Max,
        Clamp,
        Mix,
        Step,
        Smoothstep
    };
}
