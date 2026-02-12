#pragma once
#include <Core/engine_types.hpp>
#include <type_traits>
#include <utility>

namespace Engine
{
	template<typename... Args>
	struct TypesList {
	private:
		template<size_t index, typename... Rest>
		struct type_at_impl;

		template<size_t index, typename First, typename... Rest>
		struct type_at_impl<index, First, Rest...> {
			using type = typename type_at_impl<index - 1, Rest...>::type;
		};

		template<typename First, typename... Rest>
		struct type_at_impl<0, First, Rest...> {
			using type = First;
		};

		template<unsigned int N, typename... Ts>
		struct pop_front_impl;

		template<typename... Ts>
		struct pop_front_impl<0, Ts...> {
			using type = TypesList<Ts...>;
		};

		template<unsigned int N, typename Head, typename... Tail>
		struct pop_front_impl<N, Head, Tail...> {
			static_assert(N <= sizeof...(Tail) + 1, "pop_front out of bounds");
			using type = typename pop_front_impl<N - 1, Tail...>::type;
		};

		template<unsigned int N, typename Accum, typename... Ts>
		struct pop_back_impl;

		template<unsigned int N, typename... AccumArgs>
		struct pop_back_impl<N, TypesList<AccumArgs...>> {
			static_assert(N <= sizeof...(AccumArgs), "pop_back out of bounds");
			using type = typename pop_front_impl<sizeof...(AccumArgs) - N, AccumArgs...>::type;
		};

		template<unsigned int N, typename... AccumArgs, typename Head, typename... Tail>
		struct pop_back_impl<N, TypesList<AccumArgs...>, Head, Tail...> {
			using type = typename pop_back_impl<N, TypesList<AccumArgs..., Head>, Tail...>::type;
		};

	public:
		using Type = TypesList<Args...>;

		constexpr TypesList() {}
		constexpr TypesList(Args&&... values)
		    requires(sizeof...(Args) > 0)
		{}
		constexpr TypesList(const TypesList& list) {}

		template<typename Func, typename... Arguments>
		static void for_each(Func&& func, Arguments&&... args)
		{
			(func.template operator()<Args>(std::forward<Arguments>(args)...), ...);
		}

		consteval static size_t size() { return sizeof...(Args); }

		template<size_t index>
		using type_at = typename type_at_impl<index, Args...>::type;

		template<typename Type>
		static consteval bool contains()
		{
			return (std::is_same_v<Type, Args> || ...);
		}

		template<typename Type, size_t index = 0>
		static consteval int32_t index_of()
		{
			if constexpr (index == size())
			{
				return -1;
			}
			else if constexpr (std::is_same_v<Type, type_at<index>>)
			{
				return index;
			}
			else
			{
				return index_of<Type, index + 1>();
			}
		}

		template<typename... AppendArgs>
		static consteval auto push_back()
		{
			return TypesList<Args..., AppendArgs...>();
		}

		template<typename... AppendArgs>
		static consteval auto push_back(AppendArgs&&... values)
		{
			return TypesList<Args..., AppendArgs...>();
		}

		template<typename... AppendArgs>
		static consteval auto push_front()
		{
			return TypesList<AppendArgs..., Args...>();
		}

		template<typename... AppendArgs>
		static consteval auto push_front(AppendArgs&&... values)
		{
			return TypesList<AppendArgs..., Args...>();
		}

		template<unsigned int count>
		static consteval auto pop_back()
		{
			return pop_back_impl<count, Args...>::type();
		}

		template<unsigned int count>
		static consteval auto pop_front()
		{
			return pop_front_impl<count, Args...>::type();
		}

		template<typename... AppendArgs>
		static consteval TypesList<Args..., AppendArgs...> merge_back(const TypesList<AppendArgs...>& obj)
		{
			return TypesList<Args..., AppendArgs...>();
		}

		template<typename... AppendArgs>
		static consteval TypesList<Args..., AppendArgs...> merge_front(const TypesList<AppendArgs...>& obj)
		{
			return TypesList<AppendArgs..., Args...>();
		}

		template<typename Ret = void>
		using make_function = Ret (*)(Args...);

		template<typename O, typename Ret = void>
		using make_method = Ret (O::*)(Args...);

		template<typename O, typename Ret = void>
		using make_const_method = Ret (O::*)(Args...) const;
	};


	template<typename T>
	struct Signature {
		static constexpr inline bool is_function     = false;
		static constexpr inline bool is_method       = false;
		static constexpr inline bool is_const_method = false;
	};

	template<typename ReturnType, typename... ArgsType>
	struct Signature<ReturnType(ArgsType...)> {
		using Type   = ReturnType (*)(ArgsType...);
		using Return = ReturnType;
		using Args   = TypesList<ArgsType...>;

		template<typename... Additional>
		using Func = ReturnType (*)(ArgsType..., Additional...);

		template<typename Instance, typename... Additional>
		using Method = ReturnType (Instance::*)(ArgsType..., Additional...);

		template<typename Instance, typename... Additional>
		using ConstMethod = ReturnType (Instance::*)(ArgsType..., Additional...) const;

		static constexpr inline bool is_function     = true;
		static constexpr inline bool is_method       = false;
		static constexpr inline bool is_const_method = false;
	};

	template<typename ReturnType, typename... ArgsType>
	struct Signature<ReturnType (*)(ArgsType...)> {
		using Type   = ReturnType (*)(ArgsType...);
		using Return = ReturnType;
		using Args   = TypesList<ArgsType...>;

		template<typename... Additional>
		using Func = ReturnType (*)(ArgsType..., Additional...);

		template<typename Instance, typename... Additional>
		using Method = ReturnType (Instance::*)(ArgsType..., Additional...);

		template<typename Instance, typename... Additional>
		using ConstMethod = ReturnType (Instance::*)(ArgsType..., Additional...) const;

		static constexpr inline bool is_function     = true;
		static constexpr inline bool is_method       = false;
		static constexpr inline bool is_const_method = false;
	};

	template<typename ReturnType, typename... ArgsType>
	struct Signature<ReturnType (&)(ArgsType...)> {
		using Type   = ReturnType (*)(ArgsType...);
		using Return = ReturnType;
		using Args   = TypesList<ArgsType...>;

		template<typename... Additional>
		using Func = ReturnType (*)(ArgsType..., Additional...);

		template<typename Instance, typename... Additional>
		using Method = ReturnType (Instance::*)(ArgsType..., Additional...);

		template<typename Instance, typename... Additional>
		using ConstMethod = ReturnType (Instance::*)(ArgsType..., Additional...) const;

		static constexpr inline bool is_function     = true;
		static constexpr inline bool is_method       = false;
		static constexpr inline bool is_const_method = false;
	};

	template<typename ClassType, typename ReturnType, typename... ArgsType>
	struct Signature<ReturnType (ClassType::*)(ArgsType...)> {
		using Type   = ReturnType (ClassType::*)(ArgsType...);
		using Args   = TypesList<ArgsType...>;
		using Return = ReturnType;
		using Class  = ClassType;

		template<typename... Additional>
		using Func = ReturnType (*)(Class*, ArgsType..., Additional...);

		template<typename Instance = void, typename... Additional>
		using Method = ReturnType (Class::*)(ArgsType..., Additional...);

		template<typename Instance = void, typename... Additional>
		using ConstMethod = ReturnType (Class::*)(ArgsType..., Additional...) const;

		static constexpr inline bool is_function     = false;
		static constexpr inline bool is_method       = true;
		static constexpr inline bool is_const_method = false;
	};

	template<typename ClassType, typename ReturnType, typename... ArgsType>
	struct Signature<ReturnType (ClassType::*)(ArgsType...) const> {
		using Type   = ReturnType (ClassType::*)(ArgsType...) const;
		using Args   = TypesList<ArgsType...>;
		using Return = ReturnType;
		using Class  = const ClassType;

		template<typename... Additional>
		using Func = ReturnType (*)(Class*, ArgsType..., Additional...);

		template<typename Instance = void, typename... Additional>
		using Method = ReturnType (Class::*)(ArgsType..., Additional...) const;

		template<typename Instance = void, typename... Additional>
		using ConstMethod = ReturnType (Class::*)(ArgsType..., Additional...) const;

		static constexpr inline bool is_function     = false;
		static constexpr inline bool is_method       = true;
		static constexpr inline bool is_const_method = true;
	};

	// Basic function/method overloads
	template<typename Ret, typename... Args>
	constexpr auto overload_of(Ret (*func)(Args...))
	{
		return func;
	}

	template<typename Ret, typename Instance, typename... Args>
	constexpr auto overload_of(Ret (Instance::*method)(Args...))
	{
		return method;
	}

	template<typename Ret, typename Instance, typename... Args>
	constexpr auto overload_of(Ret (Instance::*method)(Args...) const)
	{
		return method;
	}

	template<typename Sig, typename... Additional>
	constexpr auto overload_of(typename Signature<Sig>::template Func<Additional...> func)
	{
		return static_cast<typename Signature<Sig>::template Func<Additional...>>(func);
	}

	template<typename Sig, typename... Additional>
	constexpr auto overload_of(typename Signature<Sig>::template Method<Additional...> method)
	{
		return method;
	}

	template<typename Sig, typename... Additional>
	constexpr auto overload_of(typename Signature<Sig>::template ConstMethod<Additional...> method)
	{
		return method;
	}

	template<typename Sig, typename Instance, typename... Additional>
	constexpr auto overload_of(typename Signature<Sig>::template Method<Instance, Additional...> method)
	{
		return method;
	}

	template<typename Sig, typename Instance, typename... Additional>
	constexpr auto overload_of(typename Signature<Sig>::template ConstMethod<Instance, Additional...> method)
	{
		return method;
	}

#define trinex_scoped_method(class_name, method_name, ...)                                                                       \
	static_cast<Engine::Signature<decltype(overload_of<__VA_ARGS__>(&class_name::method_name))>::Func<>>(                        \
	        []<typename Instance, typename... Args>(Instance* instance,                                                          \
	                                                Args... args) -> decltype(instance->method_name(args...)) {                  \
		        return instance->class_name::method_name(args...);                                                               \
	        })

#define trinex_scoped_void_method(class_name, method_name, ...)                                                                  \
	static_cast<Engine::Signature<Engine::Signature<decltype(overload_of<__VA_ARGS__>(&class_name::method_name))>::Func<>>::     \
	                    Args::template make_function<void>>(                                                                     \
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
}// namespace Engine
