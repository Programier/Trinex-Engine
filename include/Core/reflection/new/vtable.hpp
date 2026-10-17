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
}// namespace Trinex::Refl::New
