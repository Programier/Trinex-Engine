#include <Core/assert.hpp>
#include <Core/etl/algorithm.hpp>
#include <Core/etl/critical_section.hpp>
#include <Core/etl/limits.hpp>
#include <Core/math/math.hpp>
#include <Core/memory/heap.hpp>
#include <Platform/memory.hpp>
#include <tlfs/tlsf.hpp>
#include <vector>

namespace Trinex
{
	static constexpr usize s_pool_size = 64 * 1024 * 1024;

	static constexpr usize align_size(usize size, usize alignment)
	{
		return (size + alignment - 1) & ~(alignment - 1);
	}

	struct Pool
	{
		Pool* next;
		void* memory;
		usize size;
		pool_t tlsf_pool;
	};

	class Backend
	{
	private:
		tlsf_t m_allocator   = nullptr;
		Pool* m_pools        = nullptr;
		CriticalSection m_cs = {};

	private:
		struct WalkEntry
		{
			void* ptr;
			usize size;
			bool used;
		};

		struct SnapshotData
		{
			WalkEntry* entries;
			usize capacity;
			usize count;
		};

		[[noreturn]] static void allocation_failure(usize size, usize alignment)
		{
			Asserts::report_failure_fmt(nullptr, __FILE__, __LINE__, __func__,
			                            "Failed to allocate memory, size = %zu, alignment = %zu", size, alignment);
		}

		static void collect_stats(void*, size_t size, int used, void* userdata)
		{
			auto stats = static_cast<HeapAllocator::Stats*>(userdata);

			++stats->block_count;
			if (used)
			{
				stats->total_used += size;
			}
			else
			{
				++stats->free_count;
				stats->total_free += size;
				stats->largest_free = etl::max(stats->largest_free, static_cast<usize>(size));
			}
		}

		static void count_blocks(void*, size_t, int, void* userdata) { ++*static_cast<usize*>(userdata); }

		static void snapshot_blocks(void* ptr, size_t size, int used, void* userdata)
		{
			auto data = static_cast<SnapshotData*>(userdata);
			if (data->count < data->capacity)
			{
				data->entries[data->count] = {
				        .ptr  = ptr,
				        .size = size,
				        .used = used != 0,
				};
			}

			++data->count;
		}

		static constexpr usize pool_offset() { return align_size(sizeof(Pool), alignof(std::max_align_t)); }

		static void* pool_memory(Pool* pool) { return static_cast<u8*>(pool->memory) + pool_offset(); }

		static usize pool_size(Pool* pool) { return pool->size - pool_offset(); }

		static Pool* create_pool(usize size)
		{
			auto memory = Platform::Memory::instance();
			if (!memory)
				return nullptr;

			const usize alignment = tlsf_align_size();
			if (size > etl::numeric_limits<usize>::max() - tlsf_pool_overhead() - alignment)
				return nullptr;

			const usize allocation_size = Math::max(s_pool_size, size + tlsf_pool_overhead() + alignment);
			if (allocation_size > etl::numeric_limits<usize>::max() - pool_offset())
				return nullptr;

			const Platform::VirtualMemoryBlock reservation = memory->allocate(pool_offset() + allocation_size);
			if (!reservation)
				return nullptr;

			return ::new (reservation.memory) Pool{
			        .next      = nullptr,
			        .memory    = reservation.memory,
			        .size      = reservation.size,
			        .tlsf_pool = nullptr,
			};
		}

		bool add_pool(usize size)
		{
			Pool* pool = create_pool(size);
			if (!pool)
				return false;

			pool->tlsf_pool = tlsf_add_pool(m_allocator, pool_memory(pool), pool_size(pool));
			if (!pool->tlsf_pool)
			{
				Platform::Memory::instance()->release(pool->memory, pool->size);
				return false;
			}

			pool->next = m_pools;
			m_pools    = pool;
			return true;
		}

		Backend()
		{
			Pool* pool = create_pool(s_pool_size);
			if (!pool || pool_size(pool) <= tlsf_size())
				allocation_failure(s_pool_size, tlsf_align_size());

			m_allocator = tlsf_create(pool_memory(pool));
			if (m_allocator)
			{
				pool->tlsf_pool = tlsf_add_pool(m_allocator, static_cast<u8*>(pool_memory(pool)) + tlsf_size(),
				                                pool_size(pool) - tlsf_size());
			}

			if (!pool->tlsf_pool)
			{
				Platform::Memory::instance()->release(pool->memory, pool->size);
				allocation_failure(s_pool_size, tlsf_align_size());
			}

			m_pools = pool;
		}

	public:
		static Backend* instance()
		{
			static Backend backend;
			return &backend;
		}

		void* allocate(usize size)
		{
			const ScopeLock lock(m_cs);
			if (void* ptr = tlsf_malloc(m_allocator, size))
				return ptr;

			if (!add_pool(size + tlsf_align_size()))
				allocation_failure(size, tlsf_align_size());

			if (void* ptr = tlsf_malloc(m_allocator, size))
				return ptr;

			allocation_failure(size, tlsf_align_size());
		}

		void* allocate(usize size, usize alignment)
		{
			if (alignment == 0 || (alignment & (alignment - 1)) != 0)
				allocation_failure(size, alignment);

			const ScopeLock lock(m_cs);
			if (void* ptr = tlsf_memalign(m_allocator, alignment, size))
				return ptr;

			if (size > etl::numeric_limits<usize>::max() - alignment || !add_pool(size + alignment))
				allocation_failure(size, alignment);

			if (void* ptr = tlsf_memalign(m_allocator, alignment, size))
				return ptr;

			allocation_failure(size, alignment);
		}

		void free(void* ptr)
		{
			const ScopeLock lock(m_cs);
			tlsf_free(m_allocator, ptr);
		}

		void* reallocate(void* ptr, usize size)
		{
			const ScopeLock lock(m_cs);
			if (void* result = tlsf_realloc(m_allocator, ptr, size))
				return result;

			if (size == 0)
				return nullptr;

			if (!add_pool(size))
				allocation_failure(size, tlsf_align_size());

			if (void* result = tlsf_realloc(m_allocator, ptr, size))
				return result;

			allocation_failure(size, tlsf_align_size());
		}

		usize allocation_size(const void* ptr)
		{
			if (!ptr)
				return 0;

			const ScopeLock lock(m_cs);
			return tlsf_block_size(const_cast<void*>(ptr));
		}

		bool owns(const void* ptr)
		{
			if (!ptr)
				return false;

			const ScopeLock lock(m_cs);
			const auto address = reinterpret_cast<uptr>(ptr);

			for (Pool* pool = m_pools; pool; pool = pool->next)
			{
				const uptr begin = reinterpret_cast<uptr>(pool->tlsf_pool);
				const uptr end   = reinterpret_cast<uptr>(static_cast<u8*>(pool->memory) + pool->size);
				if (address >= begin && address < end)
					return true;
			}

			return false;
		}

		void check()
		{
			const ScopeLock lock(m_cs);
			trinex_verify(tlsf_check(m_allocator) == 0);

			for (Pool* pool = m_pools; pool; pool = pool->next)
				trinex_verify(tlsf_check_pool(pool->tlsf_pool) == 0);
		}

		void walk(HeapAllocator::WalkCallback callback, void* userdata)
		{
			if (!callback)
				return;

			for (;;)
			{
				usize count = 0;
				{
					const ScopeLock lock(m_cs);

					for (Pool* pool = m_pools; pool; pool = pool->next)
						tlsf_walk_pool(pool->tlsf_pool, &count_blocks, &count);
				}

				std::vector<WalkEntry> entries(count);
				SnapshotData data = {.entries = entries.data(), .capacity = entries.size(), .count = 0};

				{
					const ScopeLock lock(m_cs);

					for (Pool* pool = m_pools; pool; pool = pool->next)
						tlsf_walk_pool(pool->tlsf_pool, &snapshot_blocks, &data);
				}

				if (data.count > data.capacity)
					continue;

				for (usize index = 0; index < data.count; ++index)
				{
					const WalkEntry& entry = entries[index];
					callback(entry.ptr, entry.size, entry.used, userdata);
				}

				return;
			}
		}

		HeapAllocator::Stats stats()
		{
			HeapAllocator::Stats stats = {};
			const ScopeLock lock(m_cs);

			for (Pool* pool = m_pools; pool; pool = pool->next)
			{
				++stats.pool_count;
				tlsf_walk_pool(pool->tlsf_pool, &collect_stats, &stats);
			}

			stats.overhead = tlsf_size() + (stats.block_count + stats.pool_count) * tlsf_alloc_overhead();
			return stats;
		}
	};

	void* HeapAllocator::allocate(usize size)
	{
		return Backend::instance()->allocate(size);
	}

	void* HeapAllocator::allocate(usize size, usize alignment)
	{
		return Backend::instance()->allocate(size, alignment);
	}

	void HeapAllocator::free(void* ptr)
	{
		if (ptr)
			Backend::instance()->free(ptr);
	}

	void* HeapAllocator::reallocate(void* ptr, usize size)
	{
		return Backend::instance()->reallocate(ptr, size);
	}

	usize HeapAllocator::size_of(const void* ptr)
	{
		return Backend::instance()->allocation_size(ptr);
	}

	bool HeapAllocator::owns(const void* ptr)
	{
		return Backend::instance()->owns(ptr);
	}

	void HeapAllocator::check()
	{
		Backend::instance()->check();
	}

	void HeapAllocator::walk(WalkCallback callback, void* userdata)
	{
		Backend::instance()->walk(callback, userdata);
	}

	void HeapAllocator::dump()
	{
		struct DumpData
		{
			usize used_blocks  = 0;
			usize free_blocks  = 0;
			usize used_bytes   = 0;
			usize free_bytes   = 0;
			usize largest_free = 0;
		};

		DumpData data;

		WalkCallback callback = +[](void* ptr, usize size, bool used, void* user) {
			auto& data = *static_cast<DumpData*>(user);

			if (used)
			{
				++data.used_blocks;
				data.used_bytes += size;
				trinex_info(Log::Memory, "  [USED]  %p  %zu bytes", ptr, size);
			}
			else
			{
				++data.free_blocks;
				data.free_bytes += size;
				data.largest_free = etl::max(data.largest_free, static_cast<usize>(size));
				trinex_info(Log::Memory, "  [FREE]  %p  %zu bytes", ptr, size);
			}
		};

		trinex_info(Log::Memory, "Heap dump:");
		walk(callback, &data);

		constexpr f32 bytes_per_megabyte = 1024.0f * 1024.0f;

		trinex_info(Log::Memory, "Heap: used: %.2f MiB in %zu blocks, free: %.2f MiB in %zu blocks, largest free: %.2f MiB",
		            data.used_bytes / bytes_per_megabyte, data.used_blocks, data.free_bytes / bytes_per_megabyte,
		            data.free_blocks, data.largest_free / bytes_per_megabyte);
	}

	HeapAllocator::Stats HeapAllocator::stats()
	{
		return Backend::instance()->stats();
	}
} // namespace Trinex
