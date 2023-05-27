#include <Core/logger.hpp>
#include <Core/predef.hpp>
#include <SDL_log.h>
#include <cstdarg>
#include <cstdio>
#include <cwchar>

namespace Engine
{

    Logger& Logger::log(const char* format, ...)
    {
        return *this;
    }

    Logger& Logger::debug(const char* format, ...)
    {
        return *this;
    }

    Logger& Logger::warning(const char* format, ...)
    {
        return *this;
    }

    Logger& Logger::error(const char* format, ...)
    {
        return *this;
    }

    Logger& Logger::error(const String& msg, const MessageList& messages)
    {
        return *this;
    }

    class BasicLogger : public Logger
    {
    public:
        BasicLogger& log(const char* format, ...)
        {
#ifdef TRINEX_ENGINE_DEBUG
            va_list args;
            va_start(args, format);
            char buffer[1024];
            vsprintf(buffer, format, args);
            va_end(args);
            printf("%s\n", buffer);
#endif
            return *this;
        }

        BasicLogger& debug(const char* format, ...)
        {
#ifdef TRINEX_ENGINE_DEBUG
            va_list args;
            va_start(args, format);
            char buffer[1024];
            vsprintf(buffer, format, args);
            va_end(args);
            printf("%s\n", buffer);
#endif
            return *this;
        }

        BasicLogger& warning(const char* format, ...)
        {
#ifdef TRINEX_ENGINE_DEBUG
            va_list args;
            va_start(args, format);
            char buffer[1024];
            vsprintf(buffer, format, args);
            va_end(args);
            printf("%s\n", buffer);
#endif
            return *this;
        }


        BasicLogger& error(const char* format, ...)
        {
#ifdef TRINEX_ENGINE_DEBUG
            va_list args;
            va_start(args, format);
            char buffer[1024];
            vsprintf(buffer, format, args);
            va_end(args);
            printf("%s\n", buffer);
#endif
            return *this;
        }

        BasicLogger& error(const String& msg, const MessageList& messages)
        {
#ifdef TRINEX_ENGINE_DEBUG
            printf("%s\n", msg.c_str());

            for (auto& str : messages)
            {
                printf("\t%s", str.c_str());
            }
#endif
            return *this;
        }
    };

    static BasicLogger logger_object;
    ENGINE_EXPORT Logger* logger = &logger_object;

    ENGINE_EXPORT Logger& standart_logger()
    {
        return logger_object;
    }
}// namespace Engine
