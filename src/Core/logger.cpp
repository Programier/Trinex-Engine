#include <Core/base_engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/critical_section.hpp>
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

#include <ctime>
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
	Logger* Logger::logger = Logger::standart();

	static String dynamic_format(const char* format, va_list list)
	{
		va_list list_copy;
		va_copy(list_copy, list);

		int size = vsnprintf(nullptr, 0, format, list_copy);
		va_end(list_copy);

		if (size < 0)
		{
			return "";
		}

		String result(size, '\0');
		vsnprintf(result.data(), size + 1, format, list);
		return result;
	}

	Logger& Logger::log(const char* tag, const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		auto msg = dynamic_format(format, args);
		va_end(args);

		return log_msg(tag, msg.c_str());
	}

	Logger& Logger::debug(const char* tag, const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		auto msg = dynamic_format(format, args);
		va_end(args);

		return debug_msg(tag, msg.c_str());
	}

	Logger& Logger::warning(const char* tag, const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		auto msg = dynamic_format(format, args);
		va_end(args);

		return warning_msg(tag, msg.c_str());
	}

	Logger& Logger::error(const char* tag, const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		auto msg = dynamic_format(format, args);
		va_end(args);

		return error_msg(tag, msg.c_str());
	}

	Logger& Logger::log_msg(const char* tag, const char* msg)
	{
		return *this;
	}

	Logger& Logger::debug_msg(const char* tag, const char* msg)
	{
		return *this;
	}

	Logger& Logger::warning_msg(const char* tag, const char* msg)
	{
		return *this;
	}

	Logger& Logger::error_msg(const char* tag, const char* msg)
	{
		return *this;
	}

	Logger* Logger::null()
	{
		static Logger logger;
		return &logger;
	}

	class BasicLogger : public Logger
	{
	private:
		CriticalSection m_critical_section;

		template<typename T, typename... Args>
		bool or_operator(T first, Args... args)
		{
			return ((first == args) || ...);
		}

		BasicLogger& write_message(PrioType prio_type, const char* tag, const char* msg, FILE* out, ConsoleColor color)
		{

#if PLATFORM_ANDROID
			m_critical_section.lock();

			char buffer[64];
			sprintf(buffer, "Trinex Engine [%s]", tag);
			__android_log_print(prio_type, buffer, "%s", msg);

			m_critical_section.unlock();
#else

			char buffer[64];
			std::time_t now = std::time(nullptr);
			std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

			m_critical_section.lock();

			set_output_color(color, out);
			fprintf(out, "[%8s][%s][%s]: ", prio_type, buffer, tag);
			fprintf(out, "%s", msg);
			set_output_color(RESET_COLOR, out);
			fflush(out);

			auto len = std::strlen(msg);
			while (len > 0 && or_operator(msg[len - 1], '\0', ' ', '\t', '\r')) --len;
			if (len == 0 || msg[len - 1] != '\n')
			{
				fprintf(out, "\n");
			}

			m_critical_section.unlock();
#endif

			return *this;
		}


	public:
		BasicLogger& log_msg(const char* tag, const char* msg) override
		{
			return write_message(INFO_PRIO, tag, msg, stdout, GREEN);
		}

		BasicLogger& debug_msg(const char* tag, const char* msg) override
		{
			return write_message(DEBUG_PRIO, tag, msg, stdout, GREEN);
		}

		BasicLogger& warning_msg(const char* tag, const char* msg) override
		{
			return write_message(WARNING_PRIO, tag, msg, stdout, BLUE);
		}

		BasicLogger& error_msg(const char* tag, const char* msg) override
		{
			return write_message(ERROR_PRIO, tag, msg, stderr, RED);
		}
	};

	Logger* Logger::standart()
	{
		static BasicLogger logger_object;
		return &logger_object;
	}
}// namespace Engine
