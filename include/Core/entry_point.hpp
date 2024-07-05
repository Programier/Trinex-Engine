#pragma once

#include <Core/object.hpp>


namespace Engine
{
    class ENGINE_EXPORT EntryPoint : public Object
    {
        declare_class(EntryPoint, Object);

    public:
        virtual int_t execute();
    };
}// namespace Engine
