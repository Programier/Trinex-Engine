#pragma once
#include <Core/etl/concepts.hpp>
#include <Core/etl/memory.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/etl/utility.hpp>

namespace Trinex::Refl::New
{
	template<typename T, typename Parent>
	class NativeTypeVTable : public Parent
	{
	public:
		void construct(void* dst, usize count = 1) const override
		{
			if constexpr (etl::is_default_constructible_v<T>)
			{
				T* values = static_cast<T*>(dst);
				for (usize index = 0; index < count; ++index) etl::construct_at(values + index);
			}
			else
			{
				trinex_unreachable();
			}
		}

		void destruct(void* value, usize count = 1) const override
		{
			if constexpr (etl::is_destructible_v<T>)
			{
				T* values = static_cast<T*>(value);
				while (count != 0) etl::destroy_at(values + --count);
			}
			else
			{
				trinex_unreachable();
			}
		}

		void copy_construct(void* dst, const void* src, usize count = 1) const override
		{
			if constexpr (etl::is_copy_constructible_v<T>)
			{
				T* destination  = static_cast<T*>(dst);
				const T* source = static_cast<const T*>(src);
				for (usize index = 0; index < count; ++index) etl::construct_at(destination + index, source[index]);
			}
			else
			{
				trinex_unreachable();
			}
		}

		void move_construct(void* dst, void* src, usize count = 1) const override
		{
			if constexpr (etl::is_move_constructible_v<T>)
			{
				T* destination = static_cast<T*>(dst);
				T* source      = static_cast<T*>(src);
				for (usize index = 0; index < count; ++index) etl::construct_at(destination + index, etl::move(source[index]));
			}
			else
			{
				trinex_unreachable();
			}
		}

		void copy_assign(void* dst, const void* src, usize count = 1) const override
		{
			if constexpr (etl::is_copy_assignable_v<T>)
			{
				T* destination  = static_cast<T*>(dst);
				const T* source = static_cast<const T*>(src);
				for (usize index = 0; index < count; ++index)
				{
					destination[index] = source[index];
				}
			}
			else
			{
				trinex_unreachable();
			}
		}

		void move_assign(void* dst, void* src, usize count = 1) const override
		{
			if constexpr (etl::is_move_assignable_v<T>)
			{
				T* destination = static_cast<T*>(dst);
				T* source      = static_cast<T*>(src);
				for (usize index = 0; index < count; ++index)
				{
					destination[index] = etl::move(source[index]);
				}
			}
			else
			{
				trinex_unreachable();
			}
		}

		void swap(void* lhs, void* rhs, usize count = 1) const override
		{
			if constexpr (etl::is_swappable_v<T>)
			{
				T* left  = static_cast<T*>(lhs);
				T* right = static_cast<T*>(rhs);
				for (usize index = 0; index < count; ++index) etl::swap(left[index], right[index]);
			}
			else
			{
				trinex_unreachable();
			}
		}

		void relocate(void* dst, void* src, usize count = 1) const override
		{
			if constexpr (etl::is_trivially_copyable_v<T> && etl::is_trivially_destructible_v<T>)
			{
				etl::memmove(dst, src, sizeof(T) * count);
			}
			else if constexpr (etl::is_move_constructible_v<T> && etl::is_destructible_v<T>)
			{
				T* destination = static_cast<T*>(dst);
				T* source      = static_cast<T*>(src);
				for (usize index = 0; index < count; ++index) etl::construct_at(destination + index, etl::move(source[index]));
				while (count != 0) etl::destroy_at(source + --count);
			}
			else
			{
				trinex_unreachable();
			}
		}

		bool equals(const void* lhs, const void* rhs) const override
		{
			if constexpr (requires(const T& left, const T& right) {
				              { left == right } -> etl::convertible_to<bool>;
			              })
				return *static_cast<const T*>(lhs) == *static_cast<const T*>(rhs);
			else
				return false;
		}

		i32 compare(const void* lhs, const void* rhs) const override
		{
			if constexpr (requires(const T& left, const T& right) {
				              { left < right } -> etl::convertible_to<bool>;
				              { right < left } -> etl::convertible_to<bool>;
			              })
			{
				const T& left  = *static_cast<const T*>(lhs);
				const T& right = *static_cast<const T*>(rhs);
				return left < right ? -1 : right < left ? 1 : 0;
			}
			else
			{
				return 0;
			}
		}
	};

	template<typename T, typename Parent>
	class NativeStructVTable : public NativeTypeVTable<T, Parent>
	{
	};

	template<typename T, typename Parent>
	class NativeClassVTable : public NativeStructVTable<T, Parent>
	{
	};

	template<auto method>
	struct NativeMethodInvoker;

	template<typename Class, typename Ret, typename... Args, Ret (Class::*method)(Args...)>
	struct NativeMethodInvoker<method> {
		static Ret invoke(Class* self, Args... args) { return (self->*method)(etl::forward<Args>(args)...); }
	};

	template<typename Class, typename Ret, typename... Args, Ret (Class::*method)(Args...) const>
	struct NativeMethodInvoker<method> {
		static Ret invoke(const Class* self, Args... args) { return (self->*method)(etl::forward<Args>(args)...); }
	};

	template<auto func, typename Parent>
	class NativeFunction;

	template<typename Ret, typename... Args, Ret (*func)(Args...), typename Parent>
	class NativeFunction<func, Parent> : public Parent
	{
	private:
		template<typename...>
		struct Types {
		};

		template<typename Arg>
		using Storage = etl::conditional_t<etl::is_reference_v<Arg>, etl::add_pointer_t<etl::remove_reference_t<Arg>>, Arg>;

		static constexpr u64 align_up(u64 value, u64 alignment) { return (value + alignment - 1) & ~(alignment - 1); }

		static constexpr u64 calculate_alignment()
		{
			u64 result = 1;
			((result = result < alignof(Storage<Args>) ? alignof(Storage<Args>) : result), ...);

			if constexpr (!etl::is_void_v<Ret>)
				result = result < alignof(Ret) ? alignof(Ret) : result;

			return result;
		}

		static constexpr u64 calculate_return_offset()
		{
			u64 offset = 0;

			((offset = align_up(offset, alignof(Storage<Args>)), offset += sizeof(Storage<Args>)), ...);

			if constexpr (!etl::is_void_v<Ret>)
				offset = align_up(offset, alignof(Ret));

			return offset;
		}

		static constexpr u64 calculate_size()
		{
			u64 result = calculate_return_offset();

			if constexpr (!etl::is_void_v<Ret>)
				result += sizeof(Ret);

			return align_up(result, calculate_alignment());
		}

		template<typename Arg>
		static decltype(auto) unpack(uptr address)
		{
			using T = Storage<Arg>;

			if constexpr (etl::is_reference_v<Arg>)
				return static_cast<Arg>(**reinterpret_cast<T*>(address));
			else
				return static_cast<Arg>(*reinterpret_cast<T*>(address));
		}

		template<typename... Passed>
		static void call(uptr address, Types<>, Passed&&... passed)
		{
			if constexpr (etl::is_void_v<Ret>)
			{
				func(etl::forward<Passed>(passed)...);
			}
			else
			{
				Ret& ret = *reinterpret_cast<Ret*>(align_up(address, alignof(Ret)));
				ret      = func(etl::forward<Passed>(passed)...);
			}
		}

		template<typename Arg, typename... Rest, typename... Passed>
		static void call(uptr address, Types<Arg, Rest...>, Passed&&... passed)
		{
			using T = Storage<Arg>;
			address = align_up(address, alignof(T));
			call(address + sizeof(T), Types<Rest...>{}, etl::forward<Passed>(passed)..., unpack<Arg>(address));
		}

	public:
		using Parent::Parent;

		static constexpr u64 alignment     = calculate_alignment();
		static constexpr u64 return_offset = calculate_return_offset();
		static constexpr u64 size          = calculate_size();

		void invoke(void* context) const override { call(reinterpret_cast<uptr>(context), Types<Args...>{}); }
	};

	template<typename Class, typename Ret, typename... Args, Ret (Class::*func)(Args...), typename Parent>
	class NativeFunction<func, Parent> : public NativeFunction<NativeMethodInvoker<func>::invoke, Parent>
	{
	public:
		using Base = NativeFunction<NativeMethodInvoker<func>::invoke, Parent>;
		using Base::Base;
	};

	template<typename Class, typename Ret, typename... Args, Ret (Class::*func)(Args...) const, typename Parent>
	class NativeFunction<func, Parent> : public NativeFunction<NativeMethodInvoker<func>::invoke, Parent>
	{
	public:
		using Base = NativeFunction<NativeMethodInvoker<func>::invoke, Parent>;
		using Base::Base;
	};
}// namespace Trinex::Refl::New
