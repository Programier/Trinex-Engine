#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    ENGINE_EXPORT HashIndex memory_hash_fast(const void* memory, const size_t size, HashIndex start_hash = 0);
    ENGINE_EXPORT HashIndex memory_hash(const void* memory, const size_t size, HashIndex start_hash = 0);


    template<typename Type>
    FORCE_INLINE Type align_memory(Type in, size_t alignment)
    {
        return reinterpret_cast<Type>((reinterpret_cast<size_t>(in) + (alignment - 1)) & (~(alignment - 1)));
    }
}// namespace Engine
