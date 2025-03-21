#include <Core/etl/allocator.hpp>
#include <cstdlib>

namespace Engine
{
	unsigned char* ByteAllocator::allocate_aligned(size_type size, size_type align)
	{
		return static_cast<unsigned char*>(std::aligned_alloc(align, size));
	}

	void ByteAllocator::deallocate(unsigned char* ptr) noexcept
	{
		std::free(ptr);
	}
}// namespace Engine
