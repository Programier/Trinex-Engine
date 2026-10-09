#pragma once
#include <types.hpp>

namespace Reflector
{
	hash_t memory_hash(const void* memory, const std::size_t size, hash_t seed = {0, 0});
}// namespace Reflector
