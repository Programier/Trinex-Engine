module;
#include <cstdio>
#include <cstdlib>

#if defined(_WIN32)
	#include <Windows.h>
#endif

module Trinex.Core.Assert;
import Trinex.Core.ETL.Atomic;
import Trinex.Core.ETL.SourceLocation;

namespace Trinex::Assert
{
	static Atomic<AssertHandler> g_assert_handler{nullptr};

	static const char* kind_name(FailureKind kind) noexcept
	{
		switch (kind)
		{
			case FailureKind::Assertion: return "Assertion";
			case FailureKind::Verification: return "Verification";
			case FailureKind::Failure: return "Failure";
			case FailureKind::Unreachable: return "Unreachable";
			case FailureKind::NotImplemented: return "Not implemented";
		}

		return "Unknown";
	}

	void assert_handler(AssertHandler handler) noexcept
	{
		g_assert_handler.store(handler, etl::memory_order_release);
	}

	[[noreturn]] void report_failure(FailureKind kind, StringView message, SourceLocation location)
	{
		std::fprintf(stderr,
		             "\n"
		             "╔══════════════════════════════════════════════════════════════╗\n"
		             "║                TRINEX ENGINE ASSERTION FAILED                ║\n"
		             "╠══════════════════════════════════════════════════════════════╣\n"
		             "║ File      : %s(%u)\n"
		             "║ Function  : %s\n"
		             "║ Type      : %s\n",
		             location.file_name(), location.line(), location.function_name(), kind_name(kind));

		if (!message.empty())
		{
			std::fprintf(stderr, "║ Message   : ");
			std::fwrite(message.data(), 1, message.size(), stderr);
			std::fputc('\n', stderr);
		}

		std::fprintf(stderr, "╚══════════════════════════════════════════════════════════════╝\n\n");
		std::fflush(stderr);

		if (auto handler = g_assert_handler.load(etl::memory_order_acquire))
			handler(kind, message, location);

#if defined(_WIN32)
		if (::IsDebuggerPresent())
		{
			::DebugBreak();
		}
		else
		{
			// MessageBoxA requires a null-terminated string.
			const std::string text = message.empty() ? std::string(kind_name(kind)) : std::string(message);
			::MessageBoxA(nullptr, text.c_str(), "Trinex Engine — Assertion Failed!", MB_ICONERROR | MB_OK | MB_TOPMOST);
		}
#elif defined(__GNUC__) || defined(__clang__)
		__builtin_trap();
#endif

		std::abort();
	}
} // namespace Trinex::Assert
