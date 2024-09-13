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

	BlockAllocatorBase::size_type BlockAllocatorBase::allign_pointer(unsigned char* p, size_type align) const noexcept
	{
		uintptr_t result = reinterpret_cast<uintptr_t>(p);
		return ((align - result) % align);
	}
}// namespace Engine
