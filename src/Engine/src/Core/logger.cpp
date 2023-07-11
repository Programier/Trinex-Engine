#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cwchar>

#if PLATFORM_ANDROID
#include <android/log.h>

using PrioType = int;

static constexpr PrioType DEBUG_PRIO   = ANDROID_LOG_DEBUG;
static constexpr PrioType INFO_PRIO    = ANDROID_LOG_INFO;
static constexpr PrioType WARNING_PRIO = ANDROID_LOG_WARN;
static constexpr PrioType ERROR_PRIO   = ANDROID_LOG_ERROR;

#else

using PrioType = const char*;

static PrioType DEBUG_PRIO   = "DEBUG";
static PrioType INFO_PRIO    = "INFO";
static PrioType WARNING_PRIO = "WARNING";
static PrioType ERROR_PRIO   = "ERROR";

#endif

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

#if !PLATFORM_ANDROID
static void set_output_color(ConsoleColor color, FILE* output)
{
    fprintf(output, "%s", color);
}
#endif

#endif

namespace Engine
{
    Logger& Logger::log(const char* tag, const char* format, ...)
    {
        return *this;
    }

    Logger& Logger::debug(const char* tag, const char* format, ...)
    {
        return *this;
    }

    Logger& Logger::warning(const char* tag, const char* format, ...)
    {
        return *this;
    }

    Logger& Logger::error(const char* tag, const char* format, ...)
    {
        return *this;
    }

    Logger& Logger::error(const char* tag, const String& msg, const MessageList& messages)
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

        void write_message(PrioType prio_type, const char* tag, const char* format, va_list& args, FILE* out,
                           ConsoleColor color)
        {
#if PLATFORM_ANDROID
            __android_log_vprint(prio_type, tag, format, args);
#else
            char buffer[80];
            std::time_t now = std::time(nullptr);
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

            set_output_color(color, out);
            fprintf(out, "[%6s][%s][%s]: ", prio_type, buffer, tag);

            vfprintf(out, format, args);
            set_output_color(RESET_COLOR, out);
            fflush(out);

            auto len = std::strlen(format);
            while (len > 0 && or_operator(format[len - 1], '\0', ' ', '\t', '\r')) --len;
            if (len > 0 && format[len - 1] != '\n')
            {
                fprintf(out, "\n");
            }
#endif
        }


    public:
        BasicLogger& log(const char* tag, const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            write_message(INFO_PRIO, tag, format, args, stdout, GREEN);
            va_end(args);

            return *this;
        }

        BasicLogger& debug(const char* tag, const char* format, ...)
        {
#if TRINEX_DEBUG_BUILD
            va_list args;
            va_start(args, format);
            write_message(DEBUG_PRIO, tag, format, args, stdout, GREEN);
            va_end(args);
#endif
            return *this;
        }

        BasicLogger& warning(const char* tag, const char* format, ...)
        {
#if TRINEX_DEBUG_BUILD
            va_list args;
            va_start(args, format);
            write_message(WARNING_PRIO, tag, format, args, stdout, BLUE);
            va_end(args);
#endif
            return *this;
        }


        BasicLogger& error(const char* tag, const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            write_message(ERROR_PRIO, tag, format, args, stderr, RED);
            va_end(args);
            return *this;
        }

        BasicLogger& error(const char* tag, const String& msg, const MessageList& messages)
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
