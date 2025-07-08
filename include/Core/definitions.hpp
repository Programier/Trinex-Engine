#pragma once
#include <Core/export.hpp>

namespace Engine
{
	template<typename T>
	struct TrinexDefer {
	private:
		T func;

	public:
		TrinexDefer(const T& f) : func(f) {}
		~TrinexDefer() { func(); }
	};
}// namespace Engine

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
#define TRINEX_CONCAT_IMPL(x, y) x##y
#define TRINEX_CONCAT(x, y) TRINEX_CONCAT_IMPL(x, y)

#define constructor_template(name, ...) name(__VA_ARGS__)
#define constructor_hpp(name, ...) name(__VA_ARGS__)
#define constructor_cpp(name, ...) name::constructor_template(name, __VA_ARGS__)

#define delete_copy_constructors(class_name)                                                                                     \
	class_name(const class_name&)            = delete;                                                                           \
	class_name(class_name&&)                 = delete;                                                                           \
	class_name& operator=(class_name&&)      = delete;                                                                           \
	class_name& operator=(const class_name&) = delete

#define copy_constructors_hpp(class_name)                                                                                        \
	class_name(const class_name&);                                                                                               \
	class_name(class_name&&);                                                                                                    \
	class_name& operator=(class_name&&);                                                                                         \
	class_name& operator=(const class_name&)


#define default_copy_constructors_cpp(class_name)                                                                                \
	class_name::class_name(const class_name&)            = default;                                                              \
	class_name::class_name(class_name&&)                 = default;                                                              \
	class_name& class_name::operator=(class_name&&)      = default;                                                              \
	class_name& class_name::operator=(const class_name&) = default;

#define default_copy_constructors_scoped_cpp(scope_name, class_name)                                                             \
	scope_name::class_name::class_name(const scope_name::class_name&)                        = default;                          \
	scope_name::class_name::class_name(scope_name::class_name&&)                             = default;                          \
	scope_name::class_name& scope_name::class_name::operator=(scope_name::class_name&&)      = default;                          \
	scope_name::class_name& scope_name::class_name::operator=(const scope_name::class_name&) = default;


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


#define trinex_bitfield_enum_struct(struct_type, type)                                                                           \
	static constexpr bool is_enum          = true;                                                                               \
	static constexpr bool is_bitfield_enum = true;                                                                               \
	constexpr struct_type()                = default;                                                                            \
	constexpr struct_type(type other) : bitfield(other) {}                                                                       \
	constexpr struct_type(Enum other) : value(other) {}                                                                          \
	constexpr operator Enum() const                                                                                              \
	{                                                                                                                            \
		return value;                                                                                                            \
	}                                                                                                                            \
	constexpr inline bool operator==(Enum other) const noexcept                                                                  \
	{                                                                                                                            \
		return bitfield == type(other);                                                                                          \
	}                                                                                                                            \
	constexpr inline bool operator!=(Enum other) const noexcept                                                                  \
	{                                                                                                                            \
		return bitfield != type(other);                                                                                          \
	}                                                                                                                            \
	constexpr inline struct_type operator|(Enum other) const noexcept                                                            \
	{                                                                                                                            \
		return struct_type(bitfield | type(other));                                                                              \
	}                                                                                                                            \
	constexpr inline struct_type operator&(Enum other) const noexcept                                                            \
	{                                                                                                                            \
		return struct_type(bitfield & type(other));                                                                              \
	}                                                                                                                            \
	constexpr inline struct_type operator^(Enum other) const noexcept                                                            \
	{                                                                                                                            \
		return struct_type(bitfield ^ type(other));                                                                              \
	}                                                                                                                            \
	constexpr inline struct_type& operator|=(Enum other) noexcept                                                                \
	{                                                                                                                            \
		bitfield |= other;                                                                                                       \
		return *this;                                                                                                            \
	}                                                                                                                            \
	constexpr inline struct_type operator~() const noexcept                                                                      \
	{                                                                                                                            \
		return struct_type(~bitfield);                                                                                           \
	}                                                                                                                            \
	constexpr inline struct_type& operator&=(Enum other) noexcept                                                                \
	{                                                                                                                            \
		bitfield &= other;                                                                                                       \
		return *this;                                                                                                            \
	}                                                                                                                            \
	constexpr inline struct_type& operator^=(Enum other) noexcept                                                                \
	{                                                                                                                            \
		bitfield ^= other;                                                                                                       \
		return *this;                                                                                                            \
	}                                                                                                                            \
	union                                                                                                                        \
	{                                                                                                                            \
		type bitfield;                                                                                                           \
		Enum value;                                                                                                              \
	}

#define property_hpp(class_name, type, name, _default)                                                                           \
private:                                                                                                                         \
	type m_##name = _default;                                                                                                    \
                                                                                                                                 \
public:                                                                                                                          \
	const type& name() const;                                                                                                    \
	class_name& name(const type& value)

#define property_cpp(_class, type, name)                                                                                         \
	const type& _class::name() const                                                                                             \
	{                                                                                                                            \
		return m_##name;                                                                                                         \
	}                                                                                                                            \
                                                                                                                                 \
	_class& _class::name(const type& value)                                                                                      \
	{                                                                                                                            \
		this->m_##name = value;                                                                                                  \
		return *this;                                                                                                            \
	}

#define trinex_declare_struct(struct_name, base_name)                                                                            \
protected:                                                                                                                       \
	static Engine::Refl::Struct* m_static_struct;                                                                                \
                                                                                                                                 \
public:                                                                                                                          \
	using This  = struct_name;                                                                                                   \
	using Super = base_name;                                                                                                     \
	static void static_initialize_struct();                                                                                      \
	static class Engine::Refl::Struct* static_struct_instance();

#define trinex_declare_enum(enum_name)                                                                                           \
	static constexpr bool is_enum_reflected = true;                                                                              \
                                                                                                                                 \
private:                                                                                                                         \
	static Engine::Refl::Enum* s_enum;                                                                                           \
                                                                                                                                 \
public:                                                                                                                          \
	static inline class Engine::Refl::Enum* static_enum_instance()                                                               \
	{                                                                                                                            \
		return s_enum;                                                                                                           \
	}                                                                                                                            \
	static void static_initialize_enum()


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

#define trinex_non_copyable(class_name)                                                                                          \
	class_name(const class_name&)            = delete;                                                                           \
	class_name& operator=(const class_name&) = delete

#define trinex_non_moveable(class_name)                                                                                          \
	class_name(class_name&&)            = delete;                                                                                \
	class_name& operator=(class_name&&) = delete

#define trinex_defer Engine::TrinexDefer TRINEX_CONCAT(trinex_engine_defer, __LINE__) = [&]()
