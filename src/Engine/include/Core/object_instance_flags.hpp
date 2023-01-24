#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    enum ObjectInstanceFlags : BitMask
    {
        OI_None = 0x0000000000000000,
        OI_SkipGarbageCollection = 0x0000000000000001,
    };
}
