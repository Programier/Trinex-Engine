#pragma once
#include <Core/export.hpp>

#if defined(_WIN32)
#define PLATFORM_WINDOWS 1
#else
#define PLATFORM_WINDOWS 0
#endif

#if defined(__ANDROID__) || defined(ANDROID)
#define PLATFORM_ANDROID 1
#else
#define PLATFORM_ANDROID 0
#endif

#if defined(__linux__) && !PLATFORM_ANDROID
#define PLATFORM_LINUX 1
#else
#define PLATFORM_LINUX 0
#endif

#if !defined(PLATFORM_ANDROID) && !defined(PLATFORM_LINUX) && !defined(PLATFORM_WINDOWS)
#error "Platform doesn't support!"
#endif


#if __GNUC__
#define GNU_COMPILER
#elif __clang__
#define CLANG_COMPILER
#else
#error "Compiler doesn't support!"
#endif

#define CLASS class ENGINE_EXPORT
#define STRUCT struct ENGINE_EXPORT

#define PRELOAD_FUNCTION __attribute__((constructor))
#define ON_EXIT_FUNCTION __attribute__((destructor))


#define FILE_POS_INFO (String(__FILE__ ":") + std::to_string(__LINE__) + String(": ") + String(__PRETTY_FUNCTION__))

#define not_implemented FILE_POS_INFO + String("\n\tError: Method or function is not implemented!")


#define DISABLE_ALIGN __attribute((packed))
#define ALIGNED(value) __attribute((aligned(value)))
#define FORCE_INLINE __attribute__((always_inline)) inline

// USER SPECIFIC DEFINITIONS!
