#pragma once
#include <utility>

namespace Engine::Binder
{
	template<int index, typename T>
	struct Arg {
		static constexpr int idx = index;
		using Type               = T;
	};

	template<typename Default, int index, typename... Arguments>
	struct ArgAt;

	template<typename Default, int index>
	struct ArgAt<Default, index> {
		using Type = Default;
	};

	template<typename Default, int index, int idx, typename T, typename... Rest>
	struct ArgAt<Default, index, Arg<idx, T>, Rest...> {
		using Type = std::conditional_t<index == idx, T, typename ArgAt<Default, index, Rest...>::Type>;
	};

	template<template<class T> class TypeRemapper, auto func, typename Decl = decltype(func), typename... Override>
	struct ScriptBinderImpl;

	template<template<class T> class TypeRemapper, auto func, typename Ret, typename... Args, typename... Override>
	struct ScriptBinderImpl<TypeRemapper, func, Ret (*)(Args...), Override...> {
		using MappedRet = typename ArgAt<Ret, -1, Override...>::Type;
		using OutRet    = typename TypeRemapper<MappedRet>::RetType;

		template<std::size_t... idx>
		static consteval bool is_same_args(std::index_sequence<idx...>)
		{
			return (std::is_same_v<Args, typename TypeRemapper<typename ArgAt<Args, idx, Override...>::Type>::ArgType> && ...);
		}

		static consteval bool is_same_args() { return is_same_args(std::make_index_sequence<sizeof...(Args)>()); }

		template<typename... OverrideArgs>
		static consteval auto bind_internal()
		{
			using Func = OutRet (*)(typename TypeRemapper<OverrideArgs>::ArgType...);
			return static_cast<Func>([](typename TypeRemapper<OverrideArgs>::ArgType... args) -> OutRet {
				if constexpr (std::is_same_v<Ret, void>)
					func(TypeRemapper<OverrideArgs>::convert(args)...);
				else
					return TypeRemapper<MappedRet>::convert(func(TypeRemapper<OverrideArgs>::convert(args)...));
			});
		}

		template<std::size_t... idx>
		static consteval auto bind(std::index_sequence<idx...> seq)
		{
			return bind_internal<typename ArgAt<Args, idx, Override...>::Type...>();
		}

		static consteval auto bind()
		{
			if constexpr (std::is_same_v<OutRet, Ret> && is_same_args())
			{
				return func;
			}
			else
			{
				return bind(std::make_index_sequence<sizeof...(Args)>());
			}
		}
	};

	template<template<class T> class TypeRemapper, auto func, typename... Overrides>
	struct ScriptBinder : ScriptBinderImpl<TypeRemapper, func, decltype(func), Overrides...> {
	};
}// namespace Engine::Binder
