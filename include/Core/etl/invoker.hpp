#pragma once

namespace Trinex::etl
{
	namespace detail
	{
		template<typename T>
		struct Arg
		{
			using Storage = T;

			static T get(void* p) { return *static_cast<T*>(p); }
		};

		template<typename T>
		struct Arg<T&>
		{
			using Storage = T*;

			static T& get(void* p) { return **static_cast<T**>(p); }
		};

		template<typename T>
		struct Arg<T&&>
		{
			using Storage = T*;

			static T&& get(void* p) { return static_cast<T&&>(**static_cast<T**>(p)); }
		};

		template<usize Offset, typename... Args>
		struct InvokeArgs;

		template<usize Offset, typename T, typename... Rest>
		struct InvokeArgs<Offset, T, Rest...>
		{
			using Storage = typename Arg<T>::Storage;

			static constexpr usize aligned = (Offset + alignof(Storage) - 1) & ~(alignof(Storage) - 1);
			static constexpr usize next    = aligned + sizeof(Storage);
		};

		template<typename F>
		struct FunctionInvoker;

		template<typename R, typename... Args>
		struct FunctionInvoker<R (*)(Args...)>
		{
			template<usize Index, usize Offset, typename T, typename... Rest>
			static decltype(auto) get(void* context)
			{
				constexpr usize aligned = InvokeArgs<Offset, T>::aligned;

				if constexpr (Index == 0)
				{
					return Arg<T>::get(static_cast<unsigned char*>(context) + aligned);
				}
				else
				{
					return get<Index - 1, InvokeArgs<Offset, T>::next, Rest...>(context);
				}
			}

			template<usize... I>
			struct Indices
			{
			};

			template<usize N, usize... I>
			struct MakeIndices : MakeIndices<N - 1, N - 1, I...>
			{
			};

			template<usize... I>
			struct MakeIndices<0, I...>
			{
				using Type = Indices<I...>;
			};

			template<usize... I>
			static void execute(void* context, R (*func)(Args...), Indices<I...>)
			{
				(void) func(get<I, 0, Args...>(context)...);
			}

			static void call(void* context, R (*func)(Args...))
			{
				execute(context, func, typename MakeIndices<sizeof...(Args)>::Type{});
			}
		};

		template<auto F, typename... Other>
		struct Invoker
		{
			static void call(void* context, Other...) { FunctionInvoker<decltype(F)>::call(context, F); }
		};

		template<auto resolver, typename... Other>
		struct DynamicInvoker
		{
			static void call(void* context, Other... other)
			{
				auto func = resolver(context, other...);

				if (func)
				{
					FunctionInvoker<decltype(func)>::call(context, func);
				}
			}
		};
	} // namespace detail

	template<auto F, typename... Args>
	inline constexpr auto invoker = &detail::Invoker<F, Args...>::call;

	template<auto resolver, typename... Args>
	inline constexpr auto dynamic_invoker = &detail::DynamicInvoker<resolver, Args...>::call;
} // namespace Trinex::etl
