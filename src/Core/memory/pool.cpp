#include <Core/memory/heap.hpp>
#include <Core/memory/pool.hpp>

namespace Trinex
{
	struct PoolAllocatorEntry
	{
		PoolAllocatorEntry* next;
	};

	PoolAllocatorBase::PoolAllocatorBase(void* memory, usize stride, usize size)
	{
		add_memory(memory, stride, size);
	}

	void PoolAllocatorBase::add_memory(void* memory, usize stride, usize size)
	{
		auto count = size / stride;

		if (memory == nullptr || count == 0)
			return;

		PoolAllocatorEntry* entry = static_cast<PoolAllocatorEntry*>(memory);

		for (usize i = 1; i < count; ++i)
		{
			PoolAllocatorEntry* next = reinterpret_cast<PoolAllocatorEntry*>(reinterpret_cast<u8*>(entry) + stride);
			entry->next              = next;
			entry                    = next;
		}

		entry->next = m_objects;
		m_objects   = static_cast<PoolAllocatorEntry*>(memory);
	}

	void* PoolAllocatorBase::allocate()
	{
		if (m_objects)
		{
			PoolAllocatorEntry* object = m_objects;
			m_objects                  = object->next;
			return object;
		}

		return nullptr;
	}

	void PoolAllocatorBase::free(void* ptr)
	{
		if (ptr)
		{
			PoolAllocatorEntry* entry = static_cast<PoolAllocatorEntry*>(ptr);
			entry->next               = m_objects;
			m_objects                 = entry;
		}
	}

	DynamicPoolAllocatorBase::DynamicPoolAllocatorBase(void* memory, usize stride, usize size)
	{
		add_memory(memory, stride, size);
	}

	DynamicPoolAllocatorBase::~DynamicPoolAllocatorBase()
	{
		PoolAllocatorEntry* pool = m_pools;

		while (pool)
		{
			PoolAllocatorEntry* next = pool->next;
			HeapAllocator::free(pool);
			pool = next;
		}
	}

	void DynamicPoolAllocatorBase::add_pool(usize stride, usize page, usize align)
	{
		const usize size = stride * page + sizeof(PoolAllocatorEntry);

		auto pool  = static_cast<PoolAllocatorEntry*>(HeapAllocator::allocate(size, align));
		pool->next = m_pools;
		m_pools    = pool;

		add_memory(pool + 1, stride, size - sizeof(PoolAllocatorEntry));
	}

	void DynamicPoolAllocatorBase::add_memory(void* memory, usize stride, usize size)
	{
		auto count = size / stride;

		if (memory == nullptr || count == 0)
			return;

		PoolAllocatorEntry* entry = static_cast<PoolAllocatorEntry*>(memory);

		for (usize i = 1; i < count; ++i)
		{
			PoolAllocatorEntry* next = reinterpret_cast<PoolAllocatorEntry*>(reinterpret_cast<u8*>(entry) + stride);
			entry->next              = next;
			entry                    = next;
		}

		entry->next = m_objects;
		m_objects   = static_cast<PoolAllocatorEntry*>(memory);
	}

	void* DynamicPoolAllocatorBase::allocate(usize size, usize page, usize align)
	{
		if (m_objects == nullptr)
		{
			add_pool(size, page, align);
		}

		PoolAllocatorEntry* object = m_objects;
		m_objects                  = object->next;
		return object;
	}

	void DynamicPoolAllocatorBase::free(void* ptr)
	{
		if (ptr)
		{
			PoolAllocatorEntry* entry = static_cast<PoolAllocatorEntry*>(ptr);
			entry->next               = m_objects;
			m_objects                 = entry;
		}
	}
} // namespace Trinex
