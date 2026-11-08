#include <Core/etl/allocator.hpp>
#include <Core/memory/heap.hpp>

trinex_api void* operator new(std::size_t size, Trinex::HeapAllocatorTag) noexcept
{
	return Trinex::HeapAllocator::allocate(size);
}

trinex_api void* operator new(std::size_t size, Trinex::StackAllocatorTag) noexcept
{
	return Trinex::StackByteAllocator::allocate(size);
}

trinex_api void* operator new(std::size_t size, Trinex::FrameAllocatorTag) noexcept
{
	return Trinex::FrameByteAllocator::allocate(size);
}

trinex_api void* operator new(std::size_t size, std::align_val_t align, Trinex::HeapAllocatorTag) noexcept
{
	return Trinex::HeapAllocator::allocate(size, static_cast<Trinex::usize>(align));
}

trinex_api void* operator new(std::size_t size, std::align_val_t align, Trinex::StackAllocatorTag) noexcept
{
	return Trinex::StackByteAllocator::allocate_aligned(size, static_cast<std::size_t>(align));
}

trinex_api void* operator new(std::size_t size, std::align_val_t align, Trinex::FrameAllocatorTag) noexcept
{
	return Trinex::FrameByteAllocator::allocate_aligned(size, static_cast<std::size_t>(align));
}

trinex_api void operator delete(void* ptr, Trinex::HeapAllocatorTag) noexcept
{
	Trinex::HeapAllocator::free(ptr);
}
