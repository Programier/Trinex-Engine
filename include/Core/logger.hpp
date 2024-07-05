#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    class ENGINE_EXPORT Logger
    {
    public:
        virtual Logger& log(const char* tag, const char* format, ...);
        virtual Logger& debug(const char* tag, const char* format, ...);
        virtual Logger& warning(const char* tag, const char* format, ...);
        virtual Logger& error(const char* tag, const char* format, ...);
        virtual Logger& error(const char* tag, const String& msg, const MessageList& messages);
    };

    extern ENGINE_EXPORT Logger* logger;
    ENGINE_EXPORT Logger& standart_logger();

#if TRINEX_DEBUG_BUILD
#define debug_log(tag, format, ...) Engine::logger->debug(tag, format __VA_OPT__(, ## __VA_ARGS__))
#else
#define debug_log(tag, format, ...)
#endif
#define warn_log(tag, format, ...) Engine::logger->warning(tag, format __VA_OPT__(, ## __VA_ARGS__))
#define info_log(tag, format, ...) Engine::logger->log(tag, format __VA_OPT__(, ## __VA_ARGS__))
#define error_log(tag, format, ...) Engine::logger->error(tag, format __VA_OPT__(, ## __VA_ARGS__))
}// namespace Engine
