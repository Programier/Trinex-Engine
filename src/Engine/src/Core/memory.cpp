#include <Core/memory.hpp>
#include <bits/hash_bytes.h>
#include <cstring>

namespace Engine
{
    ENGINE_EXPORT HashIndex memory_hash_fast(const void* memory, const size_t size, HashIndex start_hash)
    {
        const byte* data = reinterpret_cast<const byte*>(memory);
        for (size_t index = 0; index < size; ++index)
        {
            start_hash = data[index] + (start_hash << 6) + (start_hash << 16) - start_hash;
        }
        return start_hash;
    }

    ENGINE_EXPORT HashIndex memory_hash(const void* memory, const size_t size, HashIndex start_hash)
    {
        return reinterpret_cast<HashIndex>(std::_Fnv_hash_bytes(memory, size, start_hash));
    }

    ENGINE_EXPORT const byte* memory_search(const byte* haystack, size_t haystack_len, const byte* needle,
                                            size_t needle_len)
    {
        if (needle_len > haystack_len)
        {
            return nullptr;
        }

        if(needle_len == 0)
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
