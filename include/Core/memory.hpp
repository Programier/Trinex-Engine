#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	ENGINE_EXPORT HashIndex memory_hash(const void* memory, const size_t size, HashIndex start_hash = 0);
	ENGINE_EXPORT const byte* memory_search(const byte* haystack, size_t haystack_len, const byte* needle, size_t needle_len);

	FORCE_INLINE constexpr size_t align_memory(size_t size, size_t alignment)
	{
		return ((size) + (alignment - 1)) & (~(alignment - 1));
	}

	template<typename Type>
	FORCE_INLINE Type align_memory(Type in, size_t alignment)
	{
		return reinterpret_cast<Type>((reinterpret_cast<size_t>(in) + (alignment - 1)) & (~(alignment - 1)));
	}

	FORCE_INLINE constexpr size_t align_up(size_t size, size_t alignment)
	{
		return (size + (alignment - 1)) & ~(alignment - 1);
	}

	FORCE_INLINE constexpr size_t align_down(size_t size, size_t alignment)
	{
		return size & ~(alignment - 1);
	}

	template<typename Type>
	FORCE_INLINE Type* align_up_ptr(Type* in, size_t alignment)
	{
		return reinterpret_cast<Type*>(align_up(reinterpret_cast<size_t>(in), alignment));
	}

	template<typename Type>
	FORCE_INLINE Type* align_down_ptr(Type* in, size_t alignment)
	{
		return reinterpret_cast<Type*>(align_down(reinterpret_cast<size_t>(in), alignment));
	}
}// namespace Engine
