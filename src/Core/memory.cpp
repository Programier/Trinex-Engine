#include <Core/memory.hpp>
#include <cstring>

namespace Engine
{
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
