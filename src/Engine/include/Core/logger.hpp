#pragma once
#include <Core/export.hpp>

namespace Engine
{
    CLASS Logger
    {
    public:
        virtual Logger& log(const char* format, ...) = 0;
    };

    extern ENGINE_EXPORT Logger* logger;
    ENGINE_EXPORT Logger& standart_logger();
}// namespace Engine
