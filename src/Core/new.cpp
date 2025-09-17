#include <Core/etl/allocator.hpp>

ENGINE_EXPORT void* operator new(std::size_t size, Engine::ByteAllocatorTag) noexcept
{
	return Engine::ByteAllocator::allocate(size);
}

ENGINE_EXPORT void* operator new(std::size_t size, Engine::StackByteAllocatorTag) noexcept
{
	return Engine::StackByteAllocator::allocate(size);
}

ENGINE_EXPORT void* operator new(std::size_t size, Engine::FrameByteAllocatorTag) noexcept
{
	return Engine::FrameByteAllocator::allocate(size);
}

ENGINE_EXPORT void* operator new(std::size_t size, std::align_val_t align, Engine::ByteAllocatorTag) noexcept
{
	return Engine::ByteAllocator::allocate_aligned(size, static_cast<std::size_t>(align));
}

ENGINE_EXPORT void* operator new(std::size_t size, std::align_val_t align, Engine::StackByteAllocatorTag) noexcept
{
	return Engine::StackByteAllocator::allocate_aligned(size, static_cast<std::size_t>(align));
}

ENGINE_EXPORT void* operator new(std::size_t size, std::align_val_t align, Engine::FrameByteAllocatorTag) noexcept
{
	return Engine::FrameByteAllocator::allocate_aligned(size, static_cast<std::size_t>(align));
}

ENGINE_EXPORT void operator delete(void* ptr, Engine::ByteAllocatorTag) noexcept
{
	Engine::ByteAllocator::deallocate(static_cast<unsigned char*>(ptr));
}
