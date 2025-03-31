#pragma once
#include <Core/export.hpp>
#include <utility>

namespace Engine
{
	struct AllocatorBase {
		template<typename U, typename... Args>
		void construct(U* p, Args&&... args)
		{
			::new ((void*) p) U(std::forward<Args>(args)...);
		}

		template<typename U>
		void destroy(U* p)
		{
			p->~U();
		}
	};

	struct ENGINE_EXPORT ByteAllocator : AllocatorBase {
		using value_type      = unsigned char;
		using pointer         = value_type*;
		using const_pointer   = const value_type*;
		using reference       = value_type&;
		using const_reference = const value_type&;
		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;

		static inline unsigned char* allocate(size_type size) { return allocate_aligned(size, 16); }
		static unsigned char* allocate_aligned(size_type size, size_type align);
		static void deallocate(unsigned char* ptr) noexcept;
	};

	struct ENGINE_EXPORT StackByteAllocator : AllocatorBase {
		class ENGINE_EXPORT Mark final
		{
		private:
			void* m_datas[2];

		public:
			Mark();
			Mark(const Mark&)            = delete;
			Mark(Mark&&)                 = delete;
			Mark& operator=(const Mark&) = delete;
			Mark& operator=(Mark&&)      = delete;
			~Mark();
		};

		using value_type      = unsigned char;
		using pointer         = value_type*;
		using const_pointer   = const value_type*;
		using reference       = value_type&;
		using const_reference = const value_type&;
		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;

		static inline unsigned char* allocate(size_type size) { return allocate_aligned(size, 16); }
		static inline void deallocate(unsigned char* ptr) noexcept {}

		static unsigned char* allocate_aligned(size_type size, size_type align);
		static void flush();
	};

	struct ENGINE_EXPORT FrameByteAllocator : AllocatorBase {
		using value_type      = unsigned char;
		using pointer         = value_type*;
		using const_pointer   = const value_type*;
		using reference       = value_type&;
		using const_reference = const value_type&;
		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;

		static inline unsigned char* allocate(size_type size) { return allocate_aligned(size, 16); }
		static inline void deallocate(unsigned char* ptr) noexcept {}

		static unsigned char* allocate_aligned(size_type size, size_type align);
		static void flush();
	};

	template<typename T, typename Type>
	struct TypedAllocator : Type {
		using value_type      = T;
		using pointer         = value_type*;
		using const_pointer   = const value_type*;
		using reference       = value_type&;
		using const_reference = const value_type&;
		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;

		template<typename U>
		struct rebind {
			using other = TypedAllocator<U, Type>;
		};

		TypedAllocator()                      = default;
		TypedAllocator(const TypedAllocator&) = default;

		template<typename U>
		TypedAllocator(const TypedAllocator<U, Type>&)
		{}

		TypedAllocator& operator=(const TypedAllocator& other) = default;

		~TypedAllocator() {}

		static pointer allocate(size_type size)
		{
			return reinterpret_cast<pointer>(Type::allocate_aligned(size * sizeof(T), alignof(T)));
		}

		static void deallocate(pointer ptr, size_type unused = 0) noexcept
		{
			static_cast<void>(unused);
			Type::deallocate(reinterpret_cast<unsigned char*>(ptr));
		}
	};


	template<typename T, typename U, typename Type>
	inline bool operator==(const TypedAllocator<T, Type>&, const TypedAllocator<U, Type>&)
	{
		return true;
	}

	template<typename T, typename U, typename Type>
	inline bool operator!=(const TypedAllocator<T, Type>&, const TypedAllocator<U, Type>&)
	{
		return false;
	}

	template<typename T>
	using Allocator = TypedAllocator<T, ByteAllocator>;

	template<typename T>
	using StackAllocator = TypedAllocator<T, StackByteAllocator>;

	template<typename T>
	using FrameAllocator = TypedAllocator<T, FrameByteAllocator>;
}// namespace Engine
