#pragma once
#include <Core/engine_types.hpp>
#include <cstring>

namespace Trinex
{
	trinex_api void* memcpy_elements(void* dst, const void* src, usize element_size, usize element_count, usize dst_stride = 0,
	                                 usize src_stride = 0);
	trinex_api void* memcpy_transform(void* dst, const void* src, usize element_count, usize dst_stride, usize src_stride,
	                                  void (*transform)(void* dst, const void* src));
	trinex_api u128 memory_hash(const void* memory, const usize size, u128 seed = 0);
	trinex_api const u8* memory_search(const u8* haystack, usize haystack_len, const u8* needle, usize needle_len);

	inline constexpr usize align_memory(usize size, usize alignment)
	{
		return ((size) + (alignment - 1)) & (~(alignment - 1));
	}

	template<typename Type>
	inline Type align_memory(Type in, usize alignment)
	{
		return reinterpret_cast<Type>((reinterpret_cast<usize>(in) + (alignment - 1)) & (~(alignment - 1)));
	}

	inline constexpr usize align_up(usize size, usize alignment)
	{
		return (size + (alignment - 1)) & ~(alignment - 1);
	}

	inline constexpr usize align_down(usize size, usize alignment)
	{
		return size & ~(alignment - 1);
	}

	template<typename Type>
	inline Type* align_up_ptr(Type* in, usize alignment)
	{
		return reinterpret_cast<Type*>(align_up(reinterpret_cast<usize>(in), alignment));
	}

	template<typename Type>
	inline Type* align_down_ptr(Type* in, usize alignment)
	{
		return reinterpret_cast<Type*>(align_down(reinterpret_cast<usize>(in), alignment));
	}

	struct HashBuilder
	{
		u128 hash = 0;

		HashBuilder(u128 seed = 0) : hash(seed) {}

		template<typename... Args>
		inline HashBuilder& add(const Args&... args)
		{
			((hash = memory_hash(&args, sizeof(Args), hash)), ...);
			return *this;
		}
	};
} // namespace Trinex
