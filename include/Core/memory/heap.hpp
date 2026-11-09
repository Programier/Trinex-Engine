#pragma once
#include <Core/etl/string.hpp>

namespace Trinex
{
	class trinex_api HeapAllocator final
	{
	public:
		struct Stats
		{
			usize page_count;
			usize abandoned_page_count;
			usize thread_count;
			usize arena_count;
			usize reserved;
			usize reserved_peak;
			usize committed;
			usize committed_peak;
			usize page_committed;
			usize used;
			usize used_peak;
			usize allocated_total;
			usize requested_total;
			usize freed_total;
			usize allocation_count;
			usize free_count;
			usize reset_total;
			usize purged_total;
			usize os_allocated_page_count;
			usize mmap_count;
			usize commit_count;
			usize reset_count;
			usize purge_count;
			usize page_search_count;
			usize profile_sample_count;
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

			void deallocate(T* ptr) noexcept { HeapAllocator::free(ptr); }
			void deallocate(T* ptr, size_type size) noexcept { HeapAllocator::free(ptr, size); }

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
		static void* allocate(usize size);
		static void* allocate(usize size, usize alignment);
		static void* zeroed(usize size);
		static void* zeroed(usize size, usize alignment);
		static void* reallocate(void* ptr, usize size);
		static void* reallocate(void* ptr, usize size, usize alignment);
		static void free(void* ptr);
		static void free(void* ptr, usize size);
		static usize size_of(const void* ptr);
		static bool owns(const void* ptr);
		static void collect(bool force = false);
		static void trim();
		static Stats stats();
		static char* stats_json();
		static void dump();

		template<typename T, usize align = alignof(T)>
		static T* allocate(usize size)
		{
			static_assert(align >= alignof(T));
			static_assert((align & (align - 1)) == 0);

			return static_cast<T*>(HeapAllocator::allocate(sizeof(T) * size, align));
		}
	};
} // namespace Trinex
