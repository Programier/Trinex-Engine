#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    class ENGINE_EXPORT ExecutableObject
    {
    public:
        virtual int_t execute() = 0;
        FORCE_INLINE virtual ~ExecutableObject(){};
    };
}// namespace Engine
