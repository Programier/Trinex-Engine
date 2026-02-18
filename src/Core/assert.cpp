#include <Core/assert.hpp>
#include <cstdio>
#include <cstdlib>
#include <printf.h>

namespace Engine::Asserts
{
	namespace
	{
		AssertHandler g_assert_handler = nullptr;
	}// namespace

	ENGINE_EXPORT void assert_handler(AssertHandler handler)
	{
		g_assert_handler = handler;
	}

	ENGINE_EXPORT void report_failure(const char* condition, const char* file, int line, const char* function,
	                                  const char* message)
	{
		std::fprintf(stderr,
		             "\n"
		             "╔══════════════════════════════════════════════════════════════╗\n"
		             "║                TRINEX ENGINE ASSERTION FAILED                ║\n"
		             "╠══════════════════════════════════════════════════════════════╣\n"
		             "║ File      : %s(%d)\n"
		             "║ Function  : %s\n",
		             file, line, function);

		if (condition && condition[0])
			std::fprintf(stderr, "║ Condition : %s\n", condition);

		if (message && message[0])
			std::fprintf(stderr, "║ Message   : %s\n", message);

		std::fprintf(stderr, "╚══════════════════════════════════════════════════════════════╝\n\n");

		if (g_assert_handler)
			g_assert_handler(condition, file, line, function, message);

#ifdef _WIN32
		if (IsDebuggerPresent())
			__debugbreak();
		else
			MessageBoxA(nullptr, message ? message : condition, "Trinex Engine — Assertion Failed!",
			            MB_ICONERROR | MB_OK | MB_TOPMOST);
#elif defined(__GNUC__) || defined(__clang__)
		__builtin_trap();
#endif

		std::abort();
	}

	ENGINE_EXPORT void report_failure_fmt(const char* condition, const char* file, int line, const char* function,
	                                      const char* format, ...)
	{
		if (format == nullptr)
		{
			report_failure(condition, file, line, function);
		}

		char buffer[2048];
		va_list args;
		va_start(args, format);

		int len = vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);

		if (len < 0 || len >= (int) sizeof(buffer) - 1)
		{
			snprintf(buffer, sizeof(buffer), "[formatted message too long or error]");
		}
		else
		{
			buffer[len] = '\0';
		}

		report_failure(condition, file, line, function, buffer);
	}
}// namespace Engine::Asserts
