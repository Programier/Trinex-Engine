#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	template<size_t size, size_t align = size>
	struct Storage {
		alignas(align) byte data[size];

		template<typename T, size_t offset>
		T& as()
		{
			static_assert(offset + sizeof(T) <= size, "Offset out of bounds");
			static_assert(align % alignof(T) == 0, "Storage alignment is not compatible with type T");
			static_assert(offset % alignof(T) == 0, "Offset is not properly aligned for type T");
			return *reinterpret_cast<T*>(data + offset);
		}

		template<typename T, size_t offset>
		const T& as() const
		{
			static_assert(offset + sizeof(T) <= size, "Offset out of bounds");
			static_assert(align % alignof(T) == 0, "Storage alignment is not compatible with type T");
			static_assert(offset % alignof(T) == 0, "Offset is not properly aligned for type T");
			return *reinterpret_cast<const T*>(data + offset);
		}
	};
}// namespace Engine
