#pragma once
#include <Core/export.hpp>
#include <Core/engine_types.hpp>

namespace Engine
{
    class ENGINE_EXPORT Logger
    {
    public:
        virtual Logger& log(const char* format, ...);
        virtual Logger& debug(const char* format, ...);
        virtual Logger& warning(const char* format, ...);
        virtual Logger& error(const char* format, ...);
        virtual Logger& error(const String& msg, const MessageList& messages);
    };

    extern ENGINE_EXPORT Logger* logger;
    ENGINE_EXPORT Logger& standart_logger();

#ifdef TRINEX_ENGINE_DEBUG
#define debug_log(...) logger->log(__VA_ARGS__)
#else
#define debug_log(...)
#endif
#define info_log(...) logger->log(__VA_ARGS__)
#define error_log(...) logger->error(__VA_ARGS__)
}// namespace Engine
