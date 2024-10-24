#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	template<typename T>
	struct SignatureParser {
		using Type = T;
	};

	template<typename ReturnType, typename... ArgsType>
	struct SignatureParser<ReturnType(ArgsType...)> {
		using Type = ReturnType (*)(ArgsType...);
	};

	template<typename Signature>
	constexpr typename SignatureParser<Signature>::Type func_of(typename SignatureParser<Signature>::Type func)
	{
		return func;
	}

	template<typename Signature, typename T>
	constexpr typename SignatureParser<Signature>::Type func_of(T func)
	{
		return static_cast<typename SignatureParser<Signature>::Type>(func);
	}

	template<typename Return, typename... Args, typename Instance>
	constexpr Return (Instance::*method_of(Return (Instance::*function)(Args...)))(Args...)
	{
		return function;
	}

	template<typename Return, typename... Args, typename Instance>
	constexpr Return (Instance::*method_of(Return (Instance::*function)(Args...) const))(Args...) const
	{
		return function;
	}

	template<typename OutType, typename... Args>
	constexpr OutType mask_of(Args&&... args)
	{
		return (static_cast<OutType>(args) | ...);
	}

	template<typename EnumType>
	FORCE_INLINE EnumerateType enum_value_of(EnumType value)
	{
		return static_cast<EnumerateType>(value);
	}

	template<typename Type, typename... Args>
	FORCE_INLINE Type* static_constructor_of(Args&&... args)
	{
		return new Type(std::forward<Args>(args)...);
	}

	template<typename Type, typename... Args>
	FORCE_INLINE void* static_void_constructor_of(Args&&... args)
	{
		return new Type(std::forward<Args>(args)...);
	}

	template<auto... values, typename ValueType>
	inline constexpr bool is_in(ValueType&& value)
	{
		return ((value == values) || ...);
	}

	template<auto... values, typename ValueType>
	inline constexpr bool is_not_in(ValueType&& value)
	{
		return ((value != values) && ...);
	}

	template<typename OutType = size_t>
	inline constexpr OutType fill_bits(byte count)
	{
		if (count == 0)
			return 0;

		size_t result = 0;

		while (count > 0)
		{
			result <<= 1;
			result += 1;
			--count;
		}

		return static_cast<OutType>(result);
	}

	template<typename FieldType, typename ClassType>
	inline size_t offset_of(FieldType ClassType::*field)
	{
		ClassType* instance = reinterpret_cast<ClassType*>(1024);
		return reinterpret_cast<size_t>(&(instance->*field)) - reinterpret_cast<size_t>(instance);
	}

	template<typename T>
	const std::decay_t<T>& default_value_of()
	{
		static T value{};
		return value;
	}
}// namespace Engine
