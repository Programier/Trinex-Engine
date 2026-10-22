#pragma once
#include <Core/etl/concepts.hpp>
#include <Core/etl/memory.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/etl/utility.hpp>

namespace Trinex::Refl::New
{
	template<typename T, typename VTable>
	class NativeTypeVTable : public VTable
	{
	public:
		template<typename... Args>
		NativeTypeVTable(Args&&... args) : VTable(etl::forward<Args>(args)...)
		{
			auto& vtable = static_cast<VTable&>(*this);

			if constexpr (etl::is_default_constructible_v<T>)
				vtable.construct = &NativeTypeVTable::construct;

			if constexpr (etl::is_destructible_v<T>)
				vtable.destruct = &NativeTypeVTable::destruct;

			if constexpr (etl::is_copy_constructible_v<T>)
				vtable.copy_construct = &NativeTypeVTable::copy_construct;

			if constexpr (etl::is_move_constructible_v<T>)
				vtable.move_construct = &NativeTypeVTable::move_construct;

			if constexpr (etl::is_copy_assignable_v<T>)
				vtable.copy_assign = &NativeTypeVTable::copy_assign;

			if constexpr (etl::is_move_assignable_v<T>)
				vtable.move_assign = &NativeTypeVTable::move_assign;

			if constexpr (etl::is_swappable_v<T>)
				vtable.swap = &NativeTypeVTable::swap;

			if constexpr ((etl::is_trivially_copyable_v<T> && etl::is_trivially_destructible_v<T>) ||
			              (etl::is_move_constructible_v<T> && etl::is_destructible_v<T>) )
				vtable.relocate = &NativeTypeVTable::relocate;

			if constexpr (etl::equality_comparable<T>)
				vtable.equals = &NativeTypeVTable::equals;

			if constexpr (etl::ordered_comparable<T>)
			{
				vtable.compare = &NativeTypeVTable::compare;
			}
		}

		template<typename Self>
		static void construct(const Self*, void* dst, usize count)
		{
			T* values = static_cast<T*>(dst);
			for (usize index = 0; index < count; ++index) etl::construct_at(values + index);
		}

		template<typename Self>
		static void destruct(const Self*, void* value, usize count)
		{
			T* values = static_cast<T*>(value);
			while (count != 0) etl::destroy_at(values + --count);
		}

		template<typename Self>
		static void copy_construct(const Self*, void* dst, const void* src, usize count)
		{
			T* destination  = static_cast<T*>(dst);
			const T* source = static_cast<const T*>(src);
			for (usize index = 0; index < count; ++index) etl::construct_at(destination + index, source[index]);
		}

		template<typename Self>
		static void move_construct(const Self*, void* dst, void* src, usize count)
		{
			T* destination = static_cast<T*>(dst);
			T* source      = static_cast<T*>(src);
			for (usize index = 0; index < count; ++index) etl::construct_at(destination + index, etl::move(source[index]));
		}

		template<typename Self>
		static void copy_assign(const Self*, void* dst, const void* src, usize count)
		{
			T* destination  = static_cast<T*>(dst);
			const T* source = static_cast<const T*>(src);
			for (usize index = 0; index < count; ++index)
			{
				destination[index] = source[index];
			}
		}

		template<typename Self>
		static void move_assign(const Self*, void* dst, void* src, usize count)
		{
			T* destination = static_cast<T*>(dst);
			T* source      = static_cast<T*>(src);
			for (usize index = 0; index < count; ++index)
			{
				destination[index] = etl::move(source[index]);
			}
		}

		template<typename Self>
		static void swap(const Self*, void* lhs, void* rhs, usize count)
		{
			T* left  = static_cast<T*>(lhs);
			T* right = static_cast<T*>(rhs);
			for (usize index = 0; index < count; ++index) etl::swap(left[index], right[index]);
		}

		template<typename Self>
		static void relocate(const Self*, void* dst, void* src, usize count)
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
		}

		template<typename Self>
		static bool equals(const Self*, const void* lhs, const void* rhs)
		{
			return *static_cast<const T*>(lhs) == *static_cast<const T*>(rhs);
		}

		template<typename Self>
		static i32 compare(const Self*, const void* lhs, const void* rhs)
		{
			const T& left  = *static_cast<const T*>(lhs);
			const T& right = *static_cast<const T*>(rhs);
			return left < right ? -1 : right < left ? 1 : 0;
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

		template<usize Index, typename... Types>
		struct ArgumentAt;

		template<usize Index, typename Arg, typename... Rest>
		struct ArgumentAt<Index, Arg, Rest...> : ArgumentAt<Index - 1, Rest...> {
		};

		template<typename Arg, typename... Rest>
		struct ArgumentAt<0, Arg, Rest...> {
			using Type = Arg;
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

		template<usize index, typename Arg, typename... Rest>
		static consteval u64 calculate_argument_offset(u64 offset = 0)
		{
			offset = align_up(offset, alignof(Storage<Arg>));

			if constexpr (index == 0)
			{
				return offset;
			}
			else
			{
				return calculate_argument_offset<index - 1, Rest...>(offset + sizeof(Storage<Arg>));
			}
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
		using ReturnType = Ret;

		template<usize Index>
		using ArgumentType = typename ArgumentAt<Index, Args...>::Type;

		static constexpr u64 alignment     = calculate_alignment();
		static constexpr u64 return_offset = calculate_return_offset();
		static constexpr u64 size          = calculate_size();

		template<usize index>
		static consteval u64 argument_offset()
		{
			static_assert(index < sizeof...(Args));
			return calculate_argument_offset<index, Args...>();
		}

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

	template<auto member, typename Base>
	class NativeMemberProperty;

	template<typename Class, typename T, T Class::* member, typename Base>
	class NativeMemberProperty<member, Base> : public Base
	{
	public:
		using Base::Base;
		using Type = T;

		void* address(void* context) override
		{
			if (context == nullptr)
				return nullptr;

			if constexpr (etl::is_const_v<T>)
				return nullptr;

			return &(static_cast<Class*>(context)->*member);
		}

		const void* load(const void* context, void* temporary = nullptr) override
		{
			(void) temporary;
			return context == nullptr ? nullptr : &(static_cast<const Class*>(context)->*member);
		}

		void store(void* context, void* object) override
		{
			if constexpr (!etl::is_const_v<T>)
			{
				if (void* destination = address(context))
					this->type()->copy_assign(destination, object);
			}
		}
	};

	template<typename T, T* property, typename Base>
	class NativeMemberProperty<property, Base> : public Base
	{
	public:
		using Base::Base;
		using Type = T;

		void* address(void*) override
		{
			if constexpr (etl::is_const_v<T>)
				return nullptr;

			return property;
		}

		const void* load(const void*, void* temporary = nullptr) override
		{
			(void) temporary;
			return property;
		}

		void store(void*, void* object) override
		{
			if constexpr (!etl::is_const_v<T>)
				this->type()->copy_assign(property, object);
		}
	};

	template<u64 offset, typename Base>
	class NativeOffsetProperty : public Base
	{
	public:
		using Base::Base;

		void* address(void* context) override
		{
			return context == nullptr ? nullptr : static_cast<void*>(static_cast<u8*>(context) + offset);
		}

		const void* load(const void* context, void* temporary = nullptr) override
		{
			(void) temporary;
			return context == nullptr ? nullptr : static_cast<const void*>(static_cast<const u8*>(context) + offset);
		}

		void store(void* context, void* object) override
		{
			if (void* destination = address(context))
				this->type()->copy_assign(destination, object);
		}
	};
}// namespace Trinex::Refl::New
