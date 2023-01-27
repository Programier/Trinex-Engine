#include <Core/logger.hpp>
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

    Logger& Logger::log(const wchar_t* format, ...)
    {
        return *this;
    }


    class BasicLogger : public Logger
    {
    public:
        BasicLogger& log(const char* format, ...)
        {
#ifdef ENGINE_DEBUG
            va_list args;
            va_start(args, format);
            char buffer[1024];
            vsprintf(buffer, format, args);
            va_end(args);
            SDL_Log("%s", buffer);
#endif
            return *this;
        }

        BasicLogger& log(const wchar_t* format, ...)
        {
#ifdef ENGINE_DEBUG
            va_list args;
            va_start(args, format);
            wchar_t buffer[1024];
            vswprintf(buffer, 1024, format, args);
            va_end(args);
            SDL_Log("%ls", buffer);
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
