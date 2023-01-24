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

#ifdef ENGINE_DEBUG
#define debug_log logger->log
#else
#define debug_log
#endif
}// namespace Engine
