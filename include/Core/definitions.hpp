#pragma once

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
	#define TRINEX_GNU_COMPILER
#elif __clang__
	#define TRINEX_CLANG_COMPILER
#else
	#error "Compiler doesn't support!"
#endif

#if PLATFORM_ANDROID
	#define ANDROID_API __ANDROID_API__
#else
	#define ANDROID_API 0
#endif

#define STRUCT_OFFSET(StructClass, Member) (reinterpret_cast<size_t>(&reinterpret_cast<StructClass*>(0)->Member))

#ifdef __cpp_rtti
	#define TRINEX_WITH_RTTI 1
#else
	#define TRINEX_WITH_RTTI 0
#endif

#define MAKE_ENTITY_FULL_NAME(entity_name, ...) #__VA_ARGS__ __VA_OPT__("::") #entity_name
#define ENTITY_INITIALIZER_NAME(entity_name, ...) MAKE_ENTITY_FULL_NAME(entity_name, __VA_ARGS__)

#define CONCAT_TYPE_AND_NAMESPACE(object_type, ...) __VA_ARGS__ __VA_OPT__(::) object_type

#define TRINEX_WRAP_CODE(code) code
#define TRINEX_CONCAT_IMPL(x, y) x##y
#define TRINEX_CONCAT(x, y) TRINEX_CONCAT_IMPL(x, y)

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

#define trinex_old_struct(struct_name, base_name)                                                                                \
protected:                                                                                                                       \
	static Trinex::Refl::Struct* m_static_struct;                                                                                \
                                                                                                                                 \
public:                                                                                                                          \
	using This  = struct_name;                                                                                                   \
	using Super = base_name;                                                                                                     \
	static void static_initialize_struct();                                                                                      \
	static class Trinex::Refl::Struct* static_reflection();

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

#define trinex_reflection_body(...) TRINEX_CONCAT(TRINEX_CONCAT(trinex_reflection_file_id, _), __LINE__)
#define trinex_reflect(...)
#define trinex_function(...)
#define trinex_property(...)

#define trinex_has_include(name) __has_include(<name>)

#define trinex_launder __builtin_launder
#define trinex_address_of __builtin_addressof

#define trinex_likely(x) __builtin_expect(!!(x), 1)
#define trinex_unlikely(x) __builtin_expect(!!(x), 0)

#define trinex_debug_break() __builtin_trap()

#define trinex_function_name __PRETTY_FUNCTION__

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
