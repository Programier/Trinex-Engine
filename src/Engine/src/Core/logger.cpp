#include <Core/logger.hpp>
#include <Core/predef.hpp>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cwchar>

#if PLATFORM_WINDOWS
#include <windows.h>
#define RESET_COLOR (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define RED (FOREGROUND_RED)
#define GREEN (FOREGROUND_GREEN)
#define BLUE (FOREGROUND_BLUE)
using ConsoleColor = DWORD;

static void set_output_color(ConsoleColor color, FILE*)
{
    static HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(console, color);
}

#else

#define RESET_COLOR "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"

using ConsoleColor = const char*;

static void set_output_color(ConsoleColor color, FILE* output)
{
    fprintf(output, "%s", color);
}

#endif

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
    private:
        template<typename T, typename... Args>
        bool or_operator(T first, Args... args)
        {
            return ((first == args) || ...);
        }

        void write_message(const char* tag, const char* format, va_list& args, FILE* out, ConsoleColor color)
        {
            char buffer[80];
            std::time_t now = std::time(nullptr);
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

            set_output_color(color, out);
            fprintf(out, "[%6s][%s]: ", tag, buffer);

            vfprintf(out, format, args);
            set_output_color(RESET_COLOR, out);
            fflush(out);

            auto len = std::strlen(format);
            while (len > 0 && or_operator(format[len - 1], '\0', ' ', '\t', '\r')) --len;
            if (len > 0 && format[len - 1] != '\n')
            {
                fprintf(out, "\n");
            }
        }


    public:
        BasicLogger& log(const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            write_message("LOG", format, args, stdout, GREEN);
            va_end(args);

            return *this;
        }

        BasicLogger& debug(const char* format, ...)
        {
#ifdef TRINEX_ENGINE_DEBUG
            va_list args;
            va_start(args, format);
            write_message("DEBUG", format, args, stdout, GREEN);
            va_end(args);
#endif
            return *this;
        }

        BasicLogger& warning(const char* format, ...)
        {
#ifdef TRINEX_ENGINE_DEBUG
            va_list args;
            va_start(args, format);
            write_message("WARNING", format, args, stdout, BLUE);
            va_end(args);
#endif
            return *this;
        }


        BasicLogger& error(const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            write_message("ERROR", format, args, stderr, RED);
            va_end(args);
            return *this;
        }

        BasicLogger& error(const String& msg, const MessageList& messages)
        {
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
