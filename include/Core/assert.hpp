#pragma once

namespace Trinex::Asserts
{
	using AssertHandler = void (*)(const char* condition, const char* file, int line, const char* function, const char* message);

	ENGINE_EXPORT void assert_handler(AssertHandler handler);
	[[noreturn]] ENGINE_EXPORT void report_failure(const char* condition, const char* file, int line, const char* function,
	                                               const char* message = nullptr);
	[[noreturn]] ENGINE_EXPORT void report_failure_fmt(const char* condition, const char* file, int line, const char* function,
	                                                   const char* format = nullptr, ...);
}// namespace Trinex::Asserts


#if !defined(TRINEX_ENABLE_ASSERTS)
#if TRINEX_DEBUG_BUILD || defined(_DEBUG) || !defined(NDEBUG)
#define TRINEX_ENABLE_ASSERTS 1
#else
#define TRINEX_ENABLE_ASSERTS 0
#endif
#endif

#if TRINEX_ENABLE_ASSERTS

#define trinex_assert(condition)                                                                                                 \
	do                                                                                                                           \
	{                                                                                                                            \
		if (!(condition)) [[unlikely]]                                                                                           \
		{                                                                                                                        \
			::Trinex::Asserts::report_failure(#condition, __FILE__, __LINE__, __func__);                                         \
		}                                                                                                                        \
	} while (false)

#define trinex_assert_msg(condition, message)                                                                                    \
	do                                                                                                                           \
	{                                                                                                                            \
		if (!(condition)) [[unlikely]]                                                                                           \
		{                                                                                                                        \
			::Trinex::Asserts::report_failure(#condition, __FILE__, __LINE__, __func__, message);                                \
		}                                                                                                                        \
	} while (false)

#define trinex_assert_fmt(condition, format, ...)                                                                                \
	do                                                                                                                           \
	{                                                                                                                            \
		if (!(condition)) [[unlikely]]                                                                                           \
		{                                                                                                                        \
			::Trinex::Asserts::report_failure_fmt(#condition, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__);              \
		}                                                                                                                        \
	} while (false)

#define trinex_failure() ::Trinex::Asserts::report_failure(nullptr, __FILE__, __LINE__, __func__)
#define trinex_failure_msg(message) ::Trinex::Asserts::report_failure(nullptr, __FILE__, __LINE__, __func__, message)
#define trinex_failure_fmt(message) ::Trinex::Asserts::report_failure(nullptr, __FILE__, __LINE__, __func__, format, ##__VA_)

#else
#define trinex_assert(condition) ((void) 0)
#define trinex_assert_msg(condition, message) ((void) 0)
#define trinex_assert_fmt(condition, format, ...) ((void) 0)
#define trinex_failure() ((void) 0)
#define trinex_failure_msg(message) ((void) 0)
#define trinex_failure_fmt(message) ((void) 0)
#endif


#define trinex_verify(condition)                                                                                                 \
	do                                                                                                                           \
	{                                                                                                                            \
		if (!(condition)) [[unlikely]]                                                                                           \
		{                                                                                                                        \
			::Trinex::Asserts::report_failure(#condition, __FILE__, __LINE__, __func__);                                         \
		}                                                                                                                        \
	} while (false)

#define trinex_verify_msg(condition, message)                                                                                    \
	do                                                                                                                           \
	{                                                                                                                            \
		if (!(condition)) [[unlikely]]                                                                                           \
		{                                                                                                                        \
			::Trinex::Asserts::report_failure(#condition, __FILE__, __LINE__, __func__, message);                                \
		}                                                                                                                        \
	} while (false)

#define trinex_verify_fmt(condition, format, ...)                                                                                \
	do                                                                                                                           \
	{                                                                                                                            \
		if (!(condition)) [[unlikely]]                                                                                           \
		{                                                                                                                        \
			::Trinex::Asserts::report_failure_fmt(#condition, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__);              \
		}                                                                                                                        \
	} while (false)

#define trinex_unreachable()                                                                                                     \
	do                                                                                                                           \
	{                                                                                                                            \
		::Trinex::Asserts::report_failure("Unreachable code reached!", __FILE__, __LINE__, __func__);                            \
	} while (false)

#define trinex_unreachable_msg(message)                                                                                          \
	do                                                                                                                           \
	{                                                                                                                            \
		::Trinex::Asserts::report_failure("Unreachable code reached!", __FILE__, __LINE__, __func__, message);                   \
	} while (false)

#define trinex_unreachable_fmt(format, ...)                                                                                      \
	do                                                                                                                           \
	{                                                                                                                            \
		::Trinex::Asserts::report_failure_fmt("Unreachable code reached!", __FILE__, __LINE__, __func__, format, ##__VA_ARGS__); \
	} while (false)

#define trinex_not_implemented()                                                                                                 \
	do                                                                                                                           \
	{                                                                                                                            \
		::Trinex::Asserts::report_failure("Feature not implemented yet!", __FILE__, __LINE__, __func__);                         \
	} while (false)

#define trinex_not_implemented_msg(message)                                                                                      \
	do                                                                                                                           \
	{                                                                                                                            \
		::Trinex::Asserts::report_failure("Feature not implemented yet!", __FILE__, __LINE__, __func__, message);                \
	} while (false)

#define trinex_not_implemented_fmt(format, ...)                                                                                  \
	do                                                                                                                           \
	{                                                                                                                            \
		::Trinex::Asserts::report_failure("Feature not implemented yet!", __FILE__, __LINE__, __func__, format, ##__VA_ARGS__);  \
	} while (false)
