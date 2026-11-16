#pragma once
#include <new>

namespace Trinex
{
	struct HeapAllocatorTag
	{
	};
	struct StackAllocatorTag
	{
	};
	struct FrameAllocatorTag
	{
	};
} // namespace Trinex

trinex_api void* operator new(std::size_t size, Trinex::HeapAllocatorTag) noexcept;
trinex_api void* operator new(std::size_t size, Trinex::StackAllocatorTag) noexcept;
trinex_api void* operator new(std::size_t size, Trinex::FrameAllocatorTag) noexcept;

trinex_api void* operator new(std::size_t size, std::align_val_t align, Trinex::HeapAllocatorTag) noexcept;
trinex_api void* operator new(std::size_t size, std::align_val_t align, Trinex::StackAllocatorTag) noexcept;
trinex_api void* operator new(std::size_t size, std::align_val_t align, Trinex::FrameAllocatorTag) noexcept;
trinex_api void operator delete(void* ptr, Trinex::HeapAllocatorTag) noexcept;

namespace Trinex
{
	template<typename F>
	class HeapAllocatorDeleter
	{
	private:
		F m_destruct;

	public:
		HeapAllocatorDeleter(F destruct) : m_destruct(destruct) {}

		template<typename T>
		inline void operator=(const T* ptr) const noexcept
		{
			if (ptr)
			{
				m_destruct(ptr);
				::operator delete(const_cast<T*>(ptr), Trinex::HeapAllocatorTag{});
			}
		}
	};
} // namespace Trinex

#define trx_new new (Trinex::HeapAllocatorTag{})
#define trx_stack_new new (Trinex::StackAllocatorTag{})
#define trx_frame_new new (Trinex::FrameAllocatorTag{})
#define trx_delete Trinex::HeapAllocatorDeleter([]<typename DestructType>(DestructType* ptr) { ptr->~DestructType(); }) =
