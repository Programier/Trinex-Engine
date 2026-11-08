#pragma once

namespace Trinex
{
	class trinex_api HeapAllocator final
	{
	public:
		struct Stats
		{
			usize pool_count;   // Total pool count
			usize total_free;   // Total free bytes available
			usize largest_free; // Largest contiguous free block
			usize total_used;   // Total bytes in allocated blocks
			usize block_count;  // Total number of blocks (free + used)
			usize free_count;   // Number of free blocks (fragmentation indicator)
			usize overhead;     // Metadata overhead bytes
		};

		template<typename T>
		class Adapter
		{
		public:
			using value_type      = T;
			using pointer         = value_type*;
			using const_pointer   = const value_type*;
			using reference       = value_type&;
			using const_reference = const value_type&;
			using size_type       = usize;
			using difference_type = iptr;

			template<typename U>
			struct rebind
			{
				using other = Adapter<U>;
			};

			Adapter() noexcept = default;

			template<typename U>
			Adapter(const Adapter<U>&) noexcept
			{}

			[[nodiscard]] T* allocate(size_type count)
			{
				const size_type size = count == 0 ? 1 : count * sizeof(T);
				return static_cast<T*>(HeapAllocator::allocate(size, alignof(T)));
			}

			void deallocate(T* ptr, size_type = 0) noexcept { HeapAllocator::free(ptr); }

			template<typename U, typename... Args>
			void construct(U* ptr, Args&&... args)
			{
				::new (ptr) U(static_cast<Args&&>(args)...);
			}

			template<typename U>
			void destroy(U* ptr)
			{
				ptr->~U();
			}

			template<typename U>
			bool operator==(const Adapter<U>&) const noexcept
			{
				return true;
			}

			template<typename U>
			bool operator!=(const Adapter<U>&) const noexcept
			{
				return false;
			}
		};

	public:
		using WalkCallback = void (*)(void* ptr, usize size, bool used, void* userdata);

		static void* allocate(usize size);
		static void* allocate(usize size, usize alignment);
		static void* reallocate(void* ptr, usize size);
		static void free(void* ptr);
		static usize size_of(const void* ptr);
		static bool owns(const void* ptr);
		static void check();
		static void walk(WalkCallback callback, void* userdata = nullptr);
		static Stats stats();
	};
} // namespace Trinex
