#pragma once

#include <Core/etl/storage.hpp>

namespace Trinex::etl
{
	namespace detail
	{
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
	} // namespace detail

	template<typename T>
	struct InvokeArg
	{
		using Storage                      = T;
		static constexpr bool has_value    = true;
		static constexpr bool is_reference = false;
		static T get(void* p) { return *static_cast<T*>(p); }
		static void store(void* destination, T value) { ::new (destination) T(static_cast<T&&>(value)); }
		static void default_store(void* destination) { ::new (destination) T(); }
		static void assign(void* destination, T value) { *static_cast<T*>(destination) = static_cast<T&&>(value); }
		static void destroy(void* value) { static_cast<T*>(value)->~T(); }
	};

	template<typename T>
	struct InvokeArg<T&>
	{
		using Storage                      = T*;
		static constexpr bool has_value    = true;
		static constexpr bool is_reference = true;
		static T& get(void* p) { return **static_cast<T**>(p); }
		static void store(void* destination, T& value) { ::new (destination) Storage(&value); }
		static void default_store(void* destination) { ::new (destination) Storage(nullptr); }
		static void assign(void* destination, T& value) { *static_cast<Storage*>(destination) = &value; }
		static void destroy(void*) {}
	};

	template<typename T>
	struct InvokeArg<T&&>
	{
		using Storage                      = T*;
		static constexpr bool has_value    = true;
		static constexpr bool is_reference = true;
		static T&& get(void* p) { return static_cast<T&&>(**static_cast<T**>(p)); }
		static void store(void* destination, T&& value) { ::new (destination) Storage(&value); }
		static void default_store(void* destination) { ::new (destination) Storage(nullptr); }
		static void assign(void* destination, T&& value) { *static_cast<Storage*>(destination) = &value; }
		static void destroy(void*) {}
	};

	template<>
	struct InvokeArg<void>
	{
		using Storage                      = unsigned char;
		static constexpr bool has_value    = false;
		static constexpr bool is_reference = false;
		static void default_store(void*) {}
		static void destroy(void*) {}
	};

	template<usize offset, typename... Args>
	struct InvokeLayout;

	template<usize offset, typename T, typename... Rest>
	struct InvokeLayout<offset, T, Rest...>
	{
		using Storage = typename InvokeArg<T>::Storage;

		static constexpr usize aligned   = (offset + alignof(Storage) - 1) & ~(alignof(Storage) - 1);
		static constexpr usize next      = aligned + sizeof(Storage);
		static constexpr usize end       = InvokeLayout<next, Rest...>::end;
		static constexpr usize alignment = alignof(Storage) > InvokeLayout<next, Rest...>::alignment
		                                           ? alignof(Storage)
		                                           : InvokeLayout<next, Rest...>::alignment;
	};

	template<usize offset>
	struct InvokeLayout<offset>
	{
		static constexpr usize end       = offset;
		static constexpr usize alignment = 1;
	};

	template<typename Ret, typename... Args>
	class InvokeContext
	{
	private:
		using Result = InvokeArg<Ret>;

		static consteval usize calculate_result_offset()
		{
			if constexpr (!Result::has_value)
				return 0;
			else
				return (InvokeLayout<0, Args...>::end + alignof(typename Result::Storage) - 1)
				       & ~(alignof(typename Result::Storage) - 1);
		}

		static constexpr usize result_offset = calculate_result_offset();
		static constexpr usize size =
		        Result::has_value ? result_offset + sizeof(typename Result::Storage) : InvokeLayout<0, Args...>::end;
		static constexpr usize alignment = InvokeLayout<0, Args...>::alignment > alignof(typename Result::Storage)
		                                           ? InvokeLayout<0, Args...>::alignment
		                                           : alignof(typename Result::Storage);

		template<usize index, usize offset, typename T, typename... Rest>
		static consteval usize argument_offset()
		{
			if constexpr (index == 0)
				return InvokeLayout<offset, T>::aligned;
			else
				return argument_offset<index - 1, InvokeLayout<offset, T>::next, Rest...>();
		}

		template<usize... I>
		void construct(detail::Indices<I...>, Args... args)
		{
			(InvokeArg<Args>::store(m_storage.data + argument_offset<I, 0, Args...>(), static_cast<Args&&>(args)), ...);
		}

		template<usize... I>
		void destruct(detail::Indices<I...>)
		{
			(InvokeArg<Args>::destroy(m_storage.data + argument_offset<I, 0, Args...>()), ...);
		}

		::Trinex::Storage<size == 0 ? 1 : size, alignment> m_storage;

	public:
		explicit InvokeContext(Args... args)
		{
			Result::default_store(m_storage.data + result_offset);
			construct(typename detail::MakeIndices<sizeof...(Args)>::Type{}, static_cast<Args&&>(args)...);
		}

		template<typename Value>
		InvokeContext(Value&& result, Args... args)
		    requires(Result::has_value)
		{
			Result::store(m_storage.data + result_offset, static_cast<Value&&>(result));
			construct(typename detail::MakeIndices<sizeof...(Args)>::Type{}, static_cast<Args&&>(args)...);
		}

		InvokeContext(const InvokeContext&)            = delete;
		InvokeContext& operator=(const InvokeContext&) = delete;

		~InvokeContext()
		{
			Result::destroy(m_storage.data + result_offset);
			destruct(typename detail::MakeIndices<sizeof...(Args)>::Type{});
		}

		void* data() { return m_storage.data; }
		const void* data() const { return m_storage.data; }
		operator void*() { return data(); }
		operator const void*() const { return data(); }

		template<typename ResultType = Ret>
		const ResultType& result()
		    requires(Result::has_value)
		{
			if constexpr (Result::is_reference)
			{
				return Result::get(m_storage.data + result_offset);
			}
			else
			{
				return *m_storage.template ptr<ResultType, result_offset>();
			}
		}
	};

	namespace detail
	{

		template<typename F>
		struct FunctionInvoker;

		template<typename R, typename... Args>
		struct FunctionInvoker<R (*)(Args...)>
		{
			using ReturnInfo    = InvokeArg<R>;
			using ReturnStorage = typename ReturnInfo::Storage;

			static consteval usize calculate_return_offset()
			{
				if constexpr (!ReturnInfo::has_value)
				{
					return 0;
				}
				else
				{
					return (InvokeLayout<0, Args...>::end + alignof(ReturnStorage) - 1) & ~(alignof(ReturnStorage) - 1);
				}
			}

			static constexpr usize return_offset = calculate_return_offset();

			template<usize index, usize offset, typename T, typename... Rest>
			static decltype(auto) get(void* context)
			{
				constexpr usize aligned = InvokeLayout<offset, T>::aligned;

				if constexpr (index == 0)
				{
					return InvokeArg<T>::get(static_cast<unsigned char*>(context) + aligned);
				}
				else
				{
					return get<index - 1, InvokeLayout<offset, T>::next, Rest...>(context);
				}
			}

			template<usize... I>
			static void execute(void* context, R (*func)(Args...), Indices<I...>)
			{
				if constexpr (!ReturnInfo::has_value)
				{
					func(get<I, 0, Args...>(context)...);
				}
				else
				{
					ReturnInfo::assign(static_cast<unsigned char*>(context) + return_offset,
					                   func(get<I, 0, Args...>(context)...));
				}
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
