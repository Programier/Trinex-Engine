#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    class ENGINE_EXPORT ExecutableObject
    {
    public:
        virtual int_t execute();
        FORCE_INLINE virtual ~ExecutableObject(){};
    };
}// namespace Engine
