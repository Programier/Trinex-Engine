#include <Core/etl/allocator.hpp>

ENGINE_EXPORT void* operator new(std::size_t size, Trinex::ByteAllocatorTag) noexcept
{
	return Trinex::ByteAllocator::allocate(size);
}

ENGINE_EXPORT void* operator new(std::size_t size, Trinex::StackByteAllocatorTag) noexcept
{
	return Trinex::StackByteAllocator::allocate(size);
}

ENGINE_EXPORT void* operator new(std::size_t size, Trinex::FrameByteAllocatorTag) noexcept
{
	return Trinex::FrameByteAllocator::allocate(size);
}

ENGINE_EXPORT void* operator new(std::size_t size, std::align_val_t align, Trinex::ByteAllocatorTag) noexcept
{
	return Trinex::ByteAllocator::allocate_aligned(size, static_cast<std::size_t>(align));
}

ENGINE_EXPORT void* operator new(std::size_t size, std::align_val_t align, Trinex::StackByteAllocatorTag) noexcept
{
	return Trinex::StackByteAllocator::allocate_aligned(size, static_cast<std::size_t>(align));
}

ENGINE_EXPORT void* operator new(std::size_t size, std::align_val_t align, Trinex::FrameByteAllocatorTag) noexcept
{
	return Trinex::FrameByteAllocator::allocate_aligned(size, static_cast<std::size_t>(align));
}

ENGINE_EXPORT void operator delete(void* ptr, Trinex::ByteAllocatorTag) noexcept
{
	Trinex::ByteAllocator::deallocate(static_cast<unsigned char*>(ptr));
}
