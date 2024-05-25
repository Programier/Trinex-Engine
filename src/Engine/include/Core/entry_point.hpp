#pragma once

#include <Core/object.hpp>


namespace Engine
{
    class ENGINE_EXPORT EntryPoint : public Object
    {
        declare_class(EntryPoint, Object);

    public:
        virtual void init(int_t argc, char** argv);
        virtual void tick();
        virtual void terminate();
    };
}// namespace Engine
