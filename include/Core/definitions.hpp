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


#if defined(arm) || defined(__arm__) || defined(__ARM_ARCH)
#define ARCH_ARM 1
#else
#define ARCH_ARM 0
#endif

#if defined(__x86_64__) || defined(m_X64)
#define ARCH_X86_64 1
#else
#define ARCH_X86_64 0
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
#define INLINE_DEBUGGABLE inline

#if PLATFORM_ANDROID
#define ANDROID_API __ANDROID_API__
#else
#define ANDROID_API 0
#endif

#define TRINEX_EXTERNAL_LIB_INIT_FUNC(ReturnType) extern "C" FORCE_ENGINE_EXPORT ReturnType create_library_interface()
#define STRUCT_OFFSET(StructClass, Member) (reinterpret_cast<size_t>(&reinterpret_cast<StructClass*>(0)->Member))

#ifdef __cpp_rtti
#define TRINEX_WITH_RTTI 1
#else
#define TRINEX_WITH_RTTI 0
#endif

#define BIT(index) (1ULL << (index))

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define MAKE_ENTITY_FULL_NAME(entity_name, ...) #__VA_ARGS__ __VA_OPT__("::") #entity_name
#define ENTITY_INITIALIZER_NAME(entity_name, ...) MAKE_ENTITY_FULL_NAME(entity_name, __VA_ARGS__)

#define CONCAT_TYPE_AND_NAMESPACE(object_type, ...) __VA_ARGS__ __VA_OPT__(::) object_type
#define HAS_INCLUDE(include_name) __has_include(<include_name>)

#define TRINEX_WRAP_CODE(code) code

// USER SPECIFIC DEFINITIONS!

#define declare_struct(struct_name, base_name)                                                                                   \
protected:                                                                                                                       \
	static class Engine::Struct* m_static_struct;                                                                                \
                                                                                                                                 \
public:                                                                                                                          \
	using This  = struct_name;                                                                                                   \
	using Super = base_name;                                                                                                     \
	static void static_initialize_struct();                                                                                      \
	static class Engine::Struct* static_struct_instance();


#define declare_enum_operators(T)                                                                                                \
	inline constexpr bool operator!(T a)                                                                                         \
	{                                                                                                                            \
		return !(__underlying_type(T)) a;                                                                                        \
	}                                                                                                                            \
	inline constexpr T operator~(T a)                                                                                            \
	{                                                                                                                            \
		return (T) ~(__underlying_type(T)) a;                                                                                    \
	}                                                                                                                            \
	inline constexpr T operator|(T a, T b)                                                                                       \
	{                                                                                                                            \
		return (T) ((__underlying_type(T)) a | (__underlying_type(T)) b);                                                        \
	}                                                                                                                            \
	inline constexpr T operator&(T a, T b)                                                                                       \
	{                                                                                                                            \
		return (T) ((__underlying_type(T)) a & (__underlying_type(T)) b);                                                        \
	}                                                                                                                            \
	inline constexpr T operator^(T a, T b)                                                                                       \
	{                                                                                                                            \
		return (T) ((__underlying_type(T)) a ^ (__underlying_type(T)) b);                                                        \
	}                                                                                                                            \
	inline T& operator|=(T& a, T b)                                                                                              \
	{                                                                                                                            \
		return a = (T) ((__underlying_type(T)) a | (__underlying_type(T)) b);                                                    \
	}                                                                                                                            \
	inline T& operator&=(T& a, T b)                                                                                              \
	{                                                                                                                            \
		return a = (T) ((__underlying_type(T)) a & (__underlying_type(T)) b);                                                    \
	}                                                                                                                            \
	inline T& operator^=(T& a, T b)                                                                                              \
	{                                                                                                                            \
		return a = (T) ((__underlying_type(T)) a ^ (__underlying_type(T)) b);                                                    \
	}
