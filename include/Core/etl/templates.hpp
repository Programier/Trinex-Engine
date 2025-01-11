#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	template<typename... Args>
	struct SignatureArgs {
		static constexpr inline unsigned int count = sizeof...(Args);
	};

	template<typename T>
	struct SignatureParser {
		using Type = T;
	};

	template<typename ReturnType, typename... ArgsType>
	struct SignatureParser<ReturnType(ArgsType...)> {
		using Type    = ReturnType(ArgsType...);
		using TypePtr = Type*;
		using TypeRef = Type&;

		using FuncType    = ReturnType(ArgsType...);
		using FuncTypePtr = FuncType*;
		using FuncTypeRef = FuncType&;

		using VoidFuncType    = void(ArgsType...);
		using VoidFuncTypePtr = VoidFuncType*;
		using VoidFuncTypeRef = VoidFuncType&;

		using Args = SignatureArgs<ArgsType...>;

		using Return = ReturnType;
	};

	template<typename ReturnType, typename... ArgsType>
	struct SignatureParser<ReturnType (*)(ArgsType...)> {
		using Type    = ReturnType(ArgsType...);
		using TypePtr = Type*;
		using TypeRef = Type&;

		using FuncType    = ReturnType(ArgsType...);
		using FuncTypePtr = FuncType*;
		using FuncTypeRef = FuncType&;

		using VoidFuncType    = void(ArgsType...);
		using VoidFuncTypePtr = VoidFuncType*;
		using VoidFuncTypeRef = VoidFuncType&;

		using Args = SignatureArgs<ArgsType...>;

		using Return = ReturnType;
	};

	template<typename ReturnType, typename... ArgsType>
	struct SignatureParser<ReturnType (&)(ArgsType...)> {
		using Type    = ReturnType(ArgsType...);
		using TypePtr = Type*;
		using TypeRef = Type&;

		using FuncType    = ReturnType(ArgsType...);
		using FuncTypePtr = FuncType*;
		using FuncTypeRef = FuncType&;

		using VoidFuncType    = void(ArgsType...);
		using VoidFuncTypePtr = VoidFuncType*;
		using VoidFuncTypeRef = VoidFuncType&;

		using Args = SignatureArgs<ArgsType...>;

		using Return = ReturnType;
	};

	template<typename ClassType, typename ReturnType, typename... ArgsType>
	struct SignatureParser<ReturnType (ClassType::*)(ArgsType...)> {
		using Type    = ReturnType (ClassType::*)(ArgsType...);
		using TypePtr = Type*;
		using TypeRef = Type&;

		using Args = SignatureArgs<ArgsType...>;

		using FuncType    = ReturnType(ClassType*, ArgsType...);
		using FuncTypePtr = FuncType*;
		using FuncTypeRef = FuncType&;

		using VoidFuncType    = void(ClassType*, ArgsType...);
		using VoidFuncTypePtr = VoidFuncType*;
		using VoidFuncTypeRef = VoidFuncType&;

		using FuncArgs = SignatureArgs<ClassType*, ArgsType...>;

		using Return = ReturnType;
		using Class  = ClassType;
	};


	template<typename ClassType, typename ReturnType, typename... ArgsType>
	struct SignatureParser<ReturnType (ClassType::*)(ArgsType...) const> {
		using Type    = ReturnType (ClassType::*)(ArgsType...) const;
		using TypePtr = Type*;
		using TypeRef = Type&;

		using Args = SignatureArgs<ArgsType...>;

		using FuncType    = ReturnType(const ClassType*, ArgsType...);
		using FuncTypePtr = FuncType*;
		using FuncTypeRef = FuncType&;

		using VoidFuncType    = void(const ClassType*, ArgsType...);
		using VoidFuncTypePtr = VoidFuncType*;
		using VoidFuncTypeRef = VoidFuncType&;

		using FuncArgs = SignatureArgs<ClassType*, ArgsType...>;

		using Return = ReturnType;
		using Class  = ClassType;
	};


	template<typename Return, typename... Args>
	constexpr Return (*func_of(Return (*func)(Args...)))(Args...)
	{
		return func;
	}

	template<typename Signature, typename T>
	constexpr typename SignatureParser<Signature>::TypePtr func_of(T func)
	{
		return static_cast<typename SignatureParser<Signature>::TypePtr>(func);
	}

	template<typename Return, typename... Args, typename Instance>
	constexpr Return (Instance::* method_of(Return (Instance::*function)(Args...)))(Args...)
	{
		return function;
	}

	template<typename Return, typename... Args, typename Instance>
	constexpr Return (Instance::* method_of(Return (Instance::*function)(Args...) const))(Args...) const
	{
		return function;
	}

#define trinex_scoped_method(class_name, method_name, ...)                                                                       \
	static_cast<Engine::SignatureParser<decltype(method_of<__VA_ARGS__>(&class_name::method_name))>::FuncTypePtr>(               \
			[]<typename Instance, typename... Args>(Instance* instance,                                                          \
													Args... args) -> decltype(instance->method_name(args...)) {                  \
				return instance->class_name::method_name(args...);                                                               \
			})

#define trinex_scoped_void_method(class_name, method_name, ...)                                                                  \
	static_cast<Engine::SignatureParser<decltype(method_of<__VA_ARGS__>(&class_name::method_name))>::VoidFuncTypePtr>(           \
			[]<typename Instance, typename... Args>(Instance* instance, Args... args) {                                          \
				instance->class_name::method_name(args...);                                                                      \
			})

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

	template<typename... Args>
	inline constexpr bool all_of(Args&&... args)
	{
		return (args && ...);
	}

	template<typename... Args>
	inline constexpr bool any_of(Args&&... args)
	{
		return (args || ...);
	}

	template<typename FieldType, typename ClassType>
	inline size_t offset_of(FieldType ClassType::* field)
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

	template<typename Type>
	void fake_delete(Type*)
	{}

	template<typename Type>
	void delete_value(Type* value)
	{
		delete value;
	}

	template<typename Type>
	void delete_array(Type* array)
	{
		delete[] array;
	}

}// namespace Engine
