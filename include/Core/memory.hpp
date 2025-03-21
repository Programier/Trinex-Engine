#pragma once
#include <Core/engine_types.hpp>
#include <utility>

namespace Engine
{
	ENGINE_EXPORT byte* allocate_memory(size_t size, size_t alignment = 16);
	ENGINE_EXPORT void release_memory(void* ptr);

	template<typename T, typename... Args>
	T* allocate(Args&&... args)
	{
		return new (allocate_memory(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
	}

	template<typename T>
	void release(T* ptr)
	{
		ptr->~T();
		release_memory(ptr);
	}

	template<typename T, typename... Args>
	T* allocate_array(size_t count, const Args&... args)
	{
		constexpr size_t max_align = alignof(T) > alignof(size_t) ? alignof(T) : alignof(size_t);
		size_t* size_ptr           = new (allocate_memory(max_align + (sizeof(T) * count), max_align)) size_t(count);

		T* array = reinterpret_cast<T*>(reinterpret_cast<byte*>(size_ptr) + max_align);
		for (size_t i = 0; i < count; ++i) new (array + i) T(args...);
		return array;
	}

	template<typename T>
	void release_array(T* array)
	{
		constexpr size_t max_align = alignof(T) > alignof(size_t) ? alignof(T) : alignof(size_t);
		size_t* size_ptr           = reinterpret_cast<size_t*>(reinterpret_cast<byte*>(array) - max_align);
		for (size_t i = 0; i < *size_ptr; ++i) (array + i)->~T();
		release_memory(size_ptr);
	}

	ENGINE_EXPORT HashIndex memory_hash_fast(const void* memory, const size_t size, HashIndex start_hash = 0);
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
	FORCE_INLINE Type align_up(Type in, size_t alignment)
	{
		return reinterpret_cast<Type>(align_up(reinterpret_cast<size_t>(in), alignment));
	}

	template<typename Type>
	FORCE_INLINE Type align_down(Type in, size_t alignment)
	{
		return reinterpret_cast<Type>(align_down(reinterpret_cast<size_t>(in), alignment));
	}
}// namespace Engine
