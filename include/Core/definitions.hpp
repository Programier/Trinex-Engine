#pragma once

// ============================================================
// Platform
// ============================================================

#if defined(__ANDROID__)
	#define trinex_platform_android 1
	#define trinex_platform_linux 0
	#define trinex_platform_windows 0

#elif defined(_WIN32)
	#define trinex_platform_android 0
	#define trinex_platform_linux 0
	#define trinex_platform_windows 1

#elif defined(__linux__)
	#define trinex_platform_android 0
	#define trinex_platform_linux 1
	#define trinex_platform_windows 0

#else
	#error "Trinex: Unsupported platform!"
#endif

// ============================================================
// Architecture
// ============================================================

#if defined(__x86_64__) || defined(_M_X64)
	#define trinex_arch_x86_64 1
	#define trinex_arch_x86 0
	#define trinex_arch_arm64 0
	#define trinex_arch_arm32 0

#elif defined(__i386__) || defined(_M_IX86)
	#define trinex_arch_x86_64 0
	#define trinex_arch_x86 1
	#define trinex_arch_arm64 0
	#define trinex_arch_arm32 0

#elif defined(__aarch64__) || defined(_M_ARM64)
	#define trinex_arch_x86_64 0
	#define trinex_arch_x86 0
	#define trinex_arch_arm64 1
	#define trinex_arch_arm32 0

#elif defined(__arm__) || defined(_M_ARM)
	#define trinex_arch_x86_64 0
	#define trinex_arch_x86 0
	#define trinex_arch_arm64 0
	#define trinex_arch_arm32 1

#else
	#error "Trinex: Unsupported architecture!"
#endif

#define trinex_arch_arm (trinex_arch_arm32 || trinex_arch_arm64)

// ============================================================
// Compiler
// ============================================================

#if defined(__clang__)
	#define trinex_compiler_clang 1
	#define trinex_compiler_gcc 0

#elif defined(__GNUC__)
	#define trinex_compiler_clang 0
	#define trinex_compiler_gcc 1

#else
	#error "Trinex: Unsupported compiler!"
#endif

// ============================================================
// Android
// ============================================================

#if trinex_platform_android && defined(__ANDROID_API__)
	#define trinex_android_api __ANDROID_API__
#else
	#define trinex_android_api 0
#endif

// ============================================================
// Common macro
// ============================================================

#define trinex_concat_impl(x, y) x##y
#define trinex_concat(x, y) trinex_concat_impl(x, y)
#define trinex_unique trinex_concat(trinex_unique_, __LINE__)
#define trinex_unique_name(name) trinex_concat(name, __LINE__)

#define trinex_enum_struct(struct_type)                                                                                          \
	static constexpr bool is_enum          = true;                                                                               \
	static constexpr bool is_bitfield_enum = false;                                                                              \
	constexpr struct_type()                = default;                                                                            \
	constexpr struct_type(Enum other) : value(other) {}                                                                          \
	constexpr operator struct_type::Enum() const                                                                                 \
	{                                                                                                                            \
		return value;                                                                                                            \
	}                                                                                                                            \
	constexpr inline bool operator==(const struct_type& other) const noexcept                                                    \
	{                                                                                                                            \
		return value == other.value;                                                                                             \
	}                                                                                                                            \
	constexpr inline bool operator!=(const struct_type& other) const noexcept                                                    \
	{                                                                                                                            \
		return value != other.value;                                                                                             \
	}                                                                                                                            \
	constexpr inline bool operator==(Enum other) const noexcept                                                                  \
	{                                                                                                                            \
		return value == other;                                                                                                   \
	}                                                                                                                            \
	constexpr inline bool operator!=(Enum other) const noexcept                                                                  \
	{                                                                                                                            \
		return value != other;                                                                                                   \
	}                                                                                                                            \
                                                                                                                                 \
	Enum value

#define trinex_bitfield_enum_struct(struct_type, underlying_type)                                                                \
	static constexpr bool is_enum          = true;                                                                               \
	static constexpr bool is_bitfield_enum = true;                                                                               \
	constexpr struct_type()                = default;                                                                            \
	constexpr struct_type(underlying_type other) : bitfield(other) {}                                                            \
                                                                                                                                 \
	constexpr operator underlying_type() const                                                                                   \
	{                                                                                                                            \
		return bitfield;                                                                                                         \
	}                                                                                                                            \
	constexpr operator underlying_type&()                                                                                        \
	{                                                                                                                            \
		return bitfield;                                                                                                         \
	}                                                                                                                            \
	constexpr inline bool all(underlying_type mask) const noexcept                                                               \
	{                                                                                                                            \
		return (bitfield & mask) == mask;                                                                                        \
	}                                                                                                                            \
	constexpr inline bool any(underlying_type mask) const noexcept                                                               \
	{                                                                                                                            \
		return (bitfield & mask) != 0;                                                                                           \
	}                                                                                                                            \
	constexpr inline struct_type set(underlying_type mask) noexcept                                                              \
	{                                                                                                                            \
		mask = mask & (~bitfield);                                                                                               \
		bitfield |= mask;                                                                                                        \
		return mask;                                                                                                             \
	}                                                                                                                            \
	constexpr inline struct_type remove(underlying_type mask) noexcept                                                           \
	{                                                                                                                            \
		mask = mask & bitfield;                                                                                                  \
		bitfield ^= mask;                                                                                                        \
		return mask;                                                                                                             \
	}                                                                                                                            \
	constexpr inline struct_type set(underlying_type mask, bool value) noexcept                                                  \
	{                                                                                                                            \
		return value ? set(mask) : remove(mask);                                                                                 \
	}                                                                                                                            \
	constexpr inline struct_type pop() noexcept                                                                                  \
	{                                                                                                                            \
		if (bitfield == 0)                                                                                                       \
			return {};                                                                                                           \
		const underlying_type first = bitfield & -bitfield;                                                                      \
		bitfield &= bitfield - 1;                                                                                                \
		return struct_type(first);                                                                                               \
	}                                                                                                                            \
	constexpr inline struct_type first() const noexcept                                                                          \
	{                                                                                                                            \
		if (bitfield == 0)                                                                                                       \
			return {};                                                                                                           \
		const underlying_type first = bitfield & -bitfield;                                                                      \
		return struct_type(first);                                                                                               \
	}                                                                                                                            \
	constexpr inline u32 index() const noexcept                                                                                  \
	{                                                                                                                            \
		return trinex_countr_zero(bitfield);                                                                                     \
	}                                                                                                                            \
	union                                                                                                                        \
	{                                                                                                                            \
		underlying_type bitfield;                                                                                                \
		Enum value;                                                                                                              \
	}

#define trinex_enum struct

#define trinex_copyable(class_name)                                                                                              \
	class_name(const class_name&);                                                                                               \
	class_name& operator=(const class_name&)

#define trinex_moveable(class_name)                                                                                              \
	class_name(class_name&&);                                                                                                    \
	class_name& operator=(class_name&&)

#define trinex_default_copyable(class_name)                                                                                      \
	inline class_name(const class_name&)            = default;                                                                   \
	inline class_name& operator=(const class_name&) = default

#define trinex_default_moveable(class_name)                                                                                      \
	inline class_name(class_name&&)            = default;                                                                        \
	inline class_name& operator=(class_name&&) = default

#define trinex_default_comparable(class_name)                                                                                    \
	inline bool operator==(const class_name&) const = default;                                                                   \
	inline bool operator!=(const class_name&) const = default

#define trinex_non_copyable(class_name)                                                                                          \
	class_name(const class_name&)            = delete;                                                                           \
	class_name& operator=(const class_name&) = delete

#define trinex_non_moveable(class_name)                                                                                          \
	class_name(class_name&&)            = delete;                                                                                \
	class_name& operator=(class_name&&) = delete

#define trinex_this_return(x)                                                                                                    \
	x;                                                                                                                           \
	return *this

#if defined(_WIN32)
	#define trinex_export __declspec(dllexport)
	#define trinex_import __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
	#define trinex_export __attribute__((visibility("default")))
	#define trinex_import __attribute__((visibility("default")))
#else
	#define trinex_export
	#define trinex_import
#endif

#define trinex_launder __builtin_launder
#define trinex_address_of __builtin_addressof

#define trinex_likely(x) __builtin_expect(!!(x), 1)
#define trinex_unlikely(x) __builtin_expect(!!(x), 0)

#define trinex_debug_break() __builtin_trap()

#define trinex_function_signature __PRETTY_FUNCTION__

#define trinex_prefetch(ptr) __builtin_prefetch(ptr)

#define trinex_array_size(array) (sizeof(array) / sizeof(array[0]))
#define trinex_bit(index) (1ULL << (index))

#define trinex_bswap16(x) __builtin_bswap16(x)
#define trinex_bswap32(x) __builtin_bswap32(x)
#define trinex_bswap64(x) __builtin_bswap64(x)

#define trinex_countr_zero(value) static_cast<Trinex::u32>(__builtin_ctzll(value))
#define trinex_countl_zero(value) static_cast<Trinex::u32>(__builtin_clzll(value))
#define trinex_popcount(value) static_cast<Trinex::u32>(__builtin_popcountll(value))

#define trinex_force_inline inline __attribute__((always_inline))
#define trinex_noinline __attribute__((noinline))
#define trinex_weak __attribute__((weak))
#define trinex_nua [[no_unique_address]]
#define trinex_is_base_of(Base, Derived) __is_base_of(Base, Derived)
#define trinex_is_same(First, Second) __is_same(First, Second)
#define trinex_is_enum(Type) __is_enum(Type)
#define trinex_is_class(Type) __is_class(Type)
#define trinex_is_union(Type) __is_union(Type)
#define trinex_is_abstract(Type) __is_abstract(Type)
#define trinex_is_final(Type) __is_final(Type)
#define trinex_is_polymorphic(Type) __is_polymorphic(Type)
#define trinex_is_pointer(Type) __is_pointer(Type)
#define trinex_is_reference(Type) __is_reference(Type)
#define trinex_is_function(Type) __is_function(Type)
#define trinex_is_trivially_copyable(Type) __is_trivially_copyable(Type)
#define trinex_is_trivially_destructible(Type) __is_trivially_destructible(Type)

// ============================================================
// Reflection
// ============================================================

#define trinex_reflection_body(...)
#define trinex_reflect(...)
#define trinex_function(...)
#define trinex_property(...)

// ============================================================
// LifeCycle
// ============================================================
#define trinex_lifecycle_handler(stage_id, ...)                                                                                  \
	static void trinex_unique();                                                                                                 \
                                                                                                                                 \
	static const ::Trinex::LifeCycle::Registrar trinex_concat(trinex_lifecycle_registrar_, __LINE__)(stage_id, &trinex_unique,   \
	                                                                                                 {__VA_ARGS__});             \
                                                                                                                                 \
	static void trinex_unique()

#define trinex_on_testing(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Testing, __VA_ARGS__)
#define trinex_on_bootstrap(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Bootstrap, __VA_ARGS__)
#define trinex_on_register(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Register, __VA_ARGS__)
#define trinex_on_configure(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Configure, __VA_ARGS__)
#define trinex_on_initialize(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Initialize, __VA_ARGS__)
#define trinex_on_load(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Load, __VA_ARGS__)
#define trinex_on_start(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Start, __VA_ARGS__)
#define trinex_on_stop(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Stop, __VA_ARGS__)
#define trinex_on_shutdown(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Shutdown, __VA_ARGS__)
#define trinex_on_teardown(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Teardown, __VA_ARGS__)
#define trinex_on_finalize(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Finalize, __VA_ARGS__)
