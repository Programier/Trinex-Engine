#pragma once
#include <Core/export.hpp>

namespace Engine
{
    class ENGINE_EXPORT System
    {
    public:
        virtual System& init()      = 0;
        virtual System& update()    = 0;
        virtual System& terminate() = 0;
        virtual ~System();
    };
}// namespace Engine
