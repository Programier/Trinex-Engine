#include <Core/etl/allocator.hpp>
#include <Core/memory.hpp>
#include <cstring>

namespace Engine
{
	ENGINE_EXPORT byte* allocate_memory(size_t size, size_t alignment)
	{
		return ByteAllocator().allocate_aligned(size, alignment);
	}

	ENGINE_EXPORT byte* allocate_memory(size_t size, size_t alignment, const void* src)
	{
		byte* memory = allocate_memory(size, alignment);
		if (src)
			std::memcpy(memory, src, size);
		return memory;
	}

	ENGINE_EXPORT void release_memory(void* ptr)
	{
		return ByteAllocator().deallocate(static_cast<ByteAllocator::pointer>(ptr));
	}

	ENGINE_EXPORT const byte* memory_search(const byte* haystack, size_t haystack_len, const byte* needle, size_t needle_len)
	{
		if (needle_len > haystack_len)
		{
			return nullptr;
		}

		if (needle_len == 0)
			return haystack;

		for (size_t i = 0, count = haystack_len - needle_len; i <= count; ++i)
		{
			if (memcmp(haystack + i, needle, needle_len) == 0)
			{
				return haystack + i;
			}
		}

		return nullptr;
	}
}// namespace Engine
