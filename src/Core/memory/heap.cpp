#include <Core/assert.hpp>
#include <Core/memory/heap.hpp>
#include <mimalloc-stats.h>
#include <mimalloc.h>

namespace Trinex
{
	[[noreturn]] static void allocation_failure(usize size, usize alignment)
	{
		Asserts::report_failure_fmt(nullptr, __FILE__, __LINE__, __func__,
		                            "Failed to allocate memory, size = %zu, alignment = %zu", size, alignment);
	}

	static usize positive_size(int64_t value)
	{
		return value > 0 ? static_cast<usize>(value) : 0;
	}

	void* HeapAllocator::allocate(usize size)
	{
		if (void* ptr = mi_malloc(size))
			return ptr;

		allocation_failure(size, alignof(std::max_align_t));
	}

	void* HeapAllocator::allocate(usize size, usize alignment)
	{
		if (alignment == 0 || (alignment & (alignment - 1)) != 0)
			allocation_failure(size, alignment);

		if (void* ptr = mi_malloc_aligned(size, alignment))
			return ptr;

		allocation_failure(size, alignment);
	}

	void* HeapAllocator::zeroed(usize size)
	{
		if (void* ptr = mi_zalloc(size))
			return ptr;

		allocation_failure(size, alignof(std::max_align_t));
	}

	void* HeapAllocator::zeroed(usize size, usize alignment)
	{
		if (void* ptr = mi_zalloc_aligned(size, alignment))
			return ptr;

		allocation_failure(size, alignof(std::max_align_t));
	}

	void* HeapAllocator::reallocate(void* ptr, usize size)
	{
		if (size == 0)
		{
			mi_free(ptr);
			return nullptr;
		}

		if (void* result = mi_realloc(ptr, size))
			return result;

		allocation_failure(size, alignof(std::max_align_t));
	}

	void* HeapAllocator::reallocate(void* ptr, usize size, usize alignment)
	{
		if (size == 0)
		{
			mi_free(ptr);
			return nullptr;
		}

		if (void* result = mi_realloc_aligned(ptr, size, alignment))
			return result;

		allocation_failure(size, alignof(std::max_align_t));
	}

	void HeapAllocator::free(void* ptr)
	{
		mi_free(ptr);
	}

	void HeapAllocator::free(void* ptr, usize size)
	{
		mi_free_size(ptr, size);
	}

	usize HeapAllocator::size_of(const void* ptr)
	{
		return ptr ? mi_usable_size(ptr) : 0;
	}

	bool HeapAllocator::owns(const void* ptr)
	{
		return ptr && mi_is_in_heap_region(ptr);
	}

	void HeapAllocator::collect(bool force)
	{
		mi_collect(force);
	}

	void HeapAllocator::trim()
	{
		mi_collect(true);
		mi_arenas_purge();
	}

	HeapAllocator::Stats HeapAllocator::stats()
	{
		mi_stats_t_decl(mimalloc_stats);
		if (!mi_stats_get(&mimalloc_stats))
			return {};

		return {
		        .page_count              = positive_size(mimalloc_stats.pages.current),
		        .abandoned_page_count    = positive_size(mimalloc_stats.pages_abandoned.current),
		        .thread_count            = positive_size(mimalloc_stats.threads.current),
		        .arena_count             = positive_size(mimalloc_stats.arena_count.total),
		        .reserved                = positive_size(mimalloc_stats.reserved.current),
		        .reserved_peak           = positive_size(mimalloc_stats.reserved.peak),
		        .committed               = positive_size(mimalloc_stats.committed.current),
		        .committed_peak          = positive_size(mimalloc_stats.committed.peak),
		        .page_committed          = positive_size(mimalloc_stats.page_committed.current),
		        .used                    = positive_size(mimalloc_stats.allocated.current),
		        .used_peak               = positive_size(mimalloc_stats.allocated.peak),
		        .allocated_total         = positive_size(mimalloc_stats.allocated.total),
		        .requested_total         = positive_size(mimalloc_stats.malloc_requested.total),
		        .freed_total             = positive_size(mimalloc_stats.freed.total),
		        .allocation_count        = positive_size(mimalloc_stats.malloc_count.total),
		        .free_count              = positive_size(mimalloc_stats.free_count.total),
		        .reset_total             = positive_size(mimalloc_stats.reset.total),
		        .purged_total            = positive_size(mimalloc_stats.purged.total),
		        .os_allocated_page_count = positive_size(mimalloc_stats.pages_os_allocated.current),
		        .mmap_count              = positive_size(mimalloc_stats.mmap_calls.total),
		        .commit_count            = positive_size(mimalloc_stats.commit_calls.total),
		        .reset_count             = positive_size(mimalloc_stats.reset_calls.total),
		        .purge_count             = positive_size(mimalloc_stats.purge_calls.total),
		        .page_search_count       = positive_size(mimalloc_stats.page_searches_count.total),
		        .profile_sample_count    = positive_size(mimalloc_stats.profile_samples.total),
		};
	}

	char* HeapAllocator::stats_json()
	{
		return mi_stats_get_json(0, nullptr);
	}

	void HeapAllocator::dump()
	{
		mi_stats_print(nullptr);
	}
} // namespace Trinex
