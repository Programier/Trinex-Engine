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
	struct HeapAllocatorDeleter
	{
		template<typename T>
		struct strip_ref
		{
			using type = T;
		};
		template<typename T>
		struct strip_ref<T&>
		{
			using type = T;
		};
		template<typename T>
		struct strip_ref<T&&>
		{
			using type = T;
		};

		template<typename T>
		inline void operator=(const T* ptr) const noexcept
		{
			if (ptr)
			{
				ptr->~T();
				::operator delete(const_cast<T*>(ptr), Trinex::HeapAllocatorTag{});
			}
		}
	};
} // namespace Trinex

#define trx_new new (Trinex::HeapAllocatorTag{})
#define trx_stack_new new (Trinex::StackAllocatorTag{})
#define trx_frame_new new (Trinex::FrameAllocatorTag{})
#define trx_delete Trinex::HeapAllocatorDeleter() =
#define trx_delete_inline(ptr)                                                                                                   \
	do                                                                                                                           \
	{                                                                                                                            \
		if (ptr)                                                                                                                 \
		{                                                                                                                        \
			using ElemType = typename Trinex::HeapAllocatorDeleter::strip_ref<decltype(*ptr)>::type;                             \
			ptr->~ElemType();                                                                                                    \
			::operator delete(ptr, Trinex::HeapAllocatorTag{});                                                                  \
		}                                                                                                                        \
	} while (0)
