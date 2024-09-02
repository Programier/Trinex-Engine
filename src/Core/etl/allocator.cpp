#include <Core/etl/allocator.hpp>
#include <memory>

namespace Engine
{
	unsigned char* ByteAllocator::allocate(size_type size)
	{
		return std::allocator<unsigned char>().allocate(size);
	}

	void ByteAllocator::deallocate(unsigned char* ptr, size_type size) noexcept
	{
		std::allocator<unsigned char>().deallocate(ptr, size);
	}
}// namespace Engine
