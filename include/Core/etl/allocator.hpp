#pragma once
#include <Core/export.hpp>
#include <utility>

namespace Engine
{
	struct ENGINE_EXPORT ByteAllocator {
		using value_type      = unsigned char;
		using pointer         = value_type*;
		using const_pointer   = const value_type*;
		using reference       = value_type&;
		using const_reference = const value_type&;
		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;

		unsigned char* allocate(size_type size);
		void deallocate(unsigned char* ptr, size_type size) noexcept;
	};

	template<typename T>
	struct Allocator {
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
		
		~Allocator()
		{}


		T* allocate(size_type size)
		{
			return reinterpret_cast<T*>(ByteAllocator().allocate(size * sizeof(T)));
		}

		void deallocate(T* ptr, size_type size) noexcept
		{
			ByteAllocator().deallocate(reinterpret_cast<unsigned char*>(ptr), size * sizeof(T));
		}


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
