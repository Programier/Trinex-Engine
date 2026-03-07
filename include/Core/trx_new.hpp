#pragma once
#include <new>

namespace Trinex
{
	struct ByteAllocatorTag {
	};
	struct StackByteAllocatorTag {
	};
	struct FrameByteAllocatorTag {
	};
}// namespace Trinex

ENGINE_EXPORT void* operator new(std::size_t size, Trinex::ByteAllocatorTag) noexcept;
ENGINE_EXPORT void* operator new(std::size_t size, Trinex::StackByteAllocatorTag) noexcept;
ENGINE_EXPORT void* operator new(std::size_t size, Trinex::FrameByteAllocatorTag) noexcept;

ENGINE_EXPORT void* operator new(std::size_t size, std::align_val_t align, Trinex::ByteAllocatorTag) noexcept;
ENGINE_EXPORT void* operator new(std::size_t size, std::align_val_t align, Trinex::StackByteAllocatorTag) noexcept;
ENGINE_EXPORT void* operator new(std::size_t size, std::align_val_t align, Trinex::FrameByteAllocatorTag) noexcept;
ENGINE_EXPORT void operator delete(void* ptr, Trinex::ByteAllocatorTag) noexcept;

namespace Trinex
{
	struct ByteAllocatorDeleter {
		template<typename T>
		struct strip_ref {
			using type = T;
		};
		template<typename T>
		struct strip_ref<T&> {
			using type = T;
		};
		template<typename T>
		struct strip_ref<T&&> {
			using type = T;
		};

		template<typename T>
		inline void operator=(const T* ptr) const noexcept
		{
			if (ptr)
			{
				ptr->~T();
				::operator delete(const_cast<T*>(ptr), Trinex::ByteAllocatorTag{});
			}
		}
	};
}// namespace Trinex

#define trx_new new (Trinex::ByteAllocatorTag{})
#define trx_stack_new new (Trinex::StackByteAllocatorTag{})
#define trx_frame_new new (Trinex::FrameByteAllocatorTag{})
#define trx_delete Trinex::ByteAllocatorDeleter() =
#define trx_delete_inline(ptr)                                                                                                   \
	do                                                                                                                           \
	{                                                                                                                            \
		if (ptr)                                                                                                                 \
		{                                                                                                                        \
			using ElemType = typename Trinex::ByteAllocatorDeleter::strip_ref<decltype(*ptr)>::type;                             \
			ptr->~ElemType();                                                                                                    \
			::operator delete(ptr, Trinex::ByteAllocatorTag{});                                                                  \
		}                                                                                                                        \
	} while (0)
