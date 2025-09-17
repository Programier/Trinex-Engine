#pragma once
#include <new>

namespace Engine
{
	struct ByteAllocatorTag {
	};
	struct StackByteAllocatorTag {
	};
	struct FrameByteAllocatorTag {
	};
}// namespace Engine

ENGINE_EXPORT void* operator new(std::size_t size, Engine::ByteAllocatorTag) noexcept;
ENGINE_EXPORT void* operator new(std::size_t size, Engine::StackByteAllocatorTag) noexcept;
ENGINE_EXPORT void* operator new(std::size_t size, Engine::FrameByteAllocatorTag) noexcept;

ENGINE_EXPORT void* operator new(std::size_t size, std::align_val_t align, Engine::ByteAllocatorTag) noexcept;
ENGINE_EXPORT void* operator new(std::size_t size, std::align_val_t align, Engine::StackByteAllocatorTag) noexcept;
ENGINE_EXPORT void* operator new(std::size_t size, std::align_val_t align, Engine::FrameByteAllocatorTag) noexcept;
ENGINE_EXPORT void operator delete(void* ptr, Engine::ByteAllocatorTag) noexcept;

namespace Engine
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
		inline void operator=(T* ptr) const noexcept
		{
			if (ptr)
			{
				ptr->~T();
				::operator delete(ptr, Engine::ByteAllocatorTag{});
			}
		}
	};
}// namespace Engine

#define trx_new new (Engine::ByteAllocatorTag{})
#define trx_stack_new new (Engine::StackByteAllocatorTag{})
#define trx_frame_new new (Engine::FrameByteAllocatorTag{})
#define trx_delete Engine::ByteAllocatorDeleter() =
#define trx_delete_inline(ptr)                                                                                                   \
	do                                                                                                                           \
	{                                                                                                                            \
		if (ptr)                                                                                                                 \
		{                                                                                                                        \
			using ElemType = typename Engine::ByteAllocatorDeleter::strip_ref<decltype(*ptr)>::type;                             \
			ptr->~ElemType();                                                                                                    \
			::operator delete(ptr, Engine::ByteAllocatorTag{});                                                                  \
		}                                                                                                                        \
	} while (0)
