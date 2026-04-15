#pragma once
#include <Core/engine_types.hpp>

namespace Trinex
{
	template<usize size, usize align = size>
	struct Storage {
		alignas(align) u8 data[size];

		template<typename T, usize idx = 0>
		T& as()
		{
			static_assert(sizeof(T) * (idx + 1) <= size, "Idx out of bounds");
			static_assert(align % alignof(T) == 0, "Storage alignment is not compatible with type T");
			return reinterpret_cast<T*>(data)[idx];
		}

		template<typename T, usize idx = 0>
		const T& as() const
		{
			static_assert(sizeof(T) * (idx + 1) <= size, "Idx out of bounds");
			static_assert(align % alignof(T) == 0, "Storage alignment is not compatible with type T");
			return reinterpret_cast<const T*>(data)[idx];
		}

		template<typename T, usize idx = 0>
		T& reinterpret()
		{
			static_assert(sizeof(T) * (idx + 1) <= size, "Idx out of bounds");
			return reinterpret_cast<T*>(data)[idx];
		}

		template<typename T, usize idx = 0>
		const T& reinterpret() const
		{
			static_assert(sizeof(T) * (idx + 1) <= size, "Idx out of bounds");
			return reinterpret_cast<const T*>(data)[idx];
		}
	};
}// namespace Trinex
