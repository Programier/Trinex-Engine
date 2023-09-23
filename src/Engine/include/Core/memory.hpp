#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    ENGINE_EXPORT HashIndex memory_hash_fast(const void* memory, const size_t size, HashIndex start_hash = 0);
    ENGINE_EXPORT HashIndex memory_hash(const void* memory, const size_t size, HashIndex start_hash = 0);
}
