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

		inline unsigned char* allocate(size_type size) { return allocate_aligned(size, 16); }
		unsigned char* allocate_aligned(size_type size, size_type align);
		void deallocate(unsigned char* ptr) noexcept;
	};

	template<typename T>
	struct Allocator : ByteAllocator {
		using value_type      = T;
		using pointer         = value_type*;
		using const_pointer   = const value_type*;
		using reference       = value_type&;
		using const_reference = const value_type&;
		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;

		template<typename U>
		struct rebind {
			using other = Allocator<U>;
		};

		Allocator()                 = default;
		Allocator(const Allocator&) = default;

		template<typename U>
		Allocator(const Allocator<U>&)
		{}

		Allocator& operator=(const Allocator& other) = default;

		~Allocator() {}

		pointer allocate(size_type size)
		{
			return reinterpret_cast<pointer>(ByteAllocator().allocate_aligned(size * sizeof(T), alignof(T)));
		}

		void deallocate(pointer ptr, size_type size) noexcept
		{
			ByteAllocator().deallocate(reinterpret_cast<unsigned char*>(ptr));
		}
	};


	template<typename T, typename U>
	inline bool operator==(const Allocator<T>&, const Allocator<U>&)
	{
		return true;
	}

	template<typename T, typename U>
	inline bool operator!=(const Allocator<T>&, const Allocator<U>&)
	{
		return false;
	}
}// namespace Engine
