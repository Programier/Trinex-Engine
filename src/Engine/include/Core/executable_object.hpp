#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    class ENGINE_EXPORT ExecutableObject
    {
    public:
        bool destroy_after_exec = false;

        virtual int_t execute();
        FORCE_INLINE virtual ~ExecutableObject(){};
    };


    class ENGINE_EXPORT SingleTimeExecutableObject : public ExecutableObject
    {
    public:
        SingleTimeExecutableObject();
    };
}// namespace Engine
