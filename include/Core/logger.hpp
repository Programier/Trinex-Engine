#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    class ENGINE_EXPORT Logger
    {
    public:
        static Logger* logger;

        virtual Logger& log(const char* tag, const char* format, ...);
        virtual Logger& debug(const char* tag, const char* format, ...);
        virtual Logger& warning(const char* tag, const char* format, ...);
        virtual Logger& error(const char* tag, const char* format, ...);
        virtual Logger& error(const char* tag, const String& msg, const MessageList& messages);

        static Logger* null();
        static Logger* standart();
    };

#if TRINEX_DEBUG_BUILD
#define debug_log(tag, format, ...) Engine::Logger::logger->debug(tag, format __VA_OPT__(, ##__VA_ARGS__))
#else
#define debug_log(tag, format, ...)
#endif
#define warn_log(tag, format, ...) Engine::Logger::logger->warning(tag, format __VA_OPT__(, ##__VA_ARGS__))
#define info_log(tag, format, ...) Engine::Logger::logger->log(tag, format __VA_OPT__(, ##__VA_ARGS__))
#define error_log(tag, format, ...) Engine::Logger::logger->error(tag, format __VA_OPT__(, ##__VA_ARGS__))
}// namespace Engine
