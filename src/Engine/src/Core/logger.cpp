#include <Core/logger.hpp>
#include <SDL_log.h>
#include <cstdarg>
#include <cstdio>

namespace Engine
{

    class BasicLogger : public Logger
    {
    public:
        BasicLogger& log(const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            char buffer[1024];
            vsprintf(buffer, format, args);
            va_end(args);
            SDL_Log("%s", buffer);
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
