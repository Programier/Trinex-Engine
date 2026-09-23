#include <Core/etl/atomic.hpp>
#include <Core/etl/critical_section.hpp>
#include <Core/etl/vector.hpp>
#include <Core/ref_counted.hpp>

namespace Trinex
{
	struct WeakEntry {
		RefCounted* ptr = nullptr;
		u32 weak_refs   = 0;

		union
		{
			u32 next_free;
			u32 index;
		};
	};

	namespace
	{
		class WeakStorage
		{
		public:
			static constexpr u32 bad_link  = 0xFFFFFFFF;
			static constexpr u32 page_size = 1024;

		private:
			struct Page {
				WeakEntry entries[page_size];
			};

		private:
			CriticalSection m_lock;
			Vector<Page*> m_pages;
			u32 m_size = 0;
			u32 m_free = bad_link;

		public:
			CriticalSection& lock() { return m_lock; }

			static WeakStorage* instance()
			{
				static WeakStorage storage;
				return &storage;
			}

			WeakEntry* entry(u32 index)
			{
				const u32 page_index  = index / page_size;
				const u32 local_index = index % page_size;

				return &m_pages[page_index]->entries[local_index];
			}

			u32 allocate(RefCounted* object)
			{
				if (m_free != bad_link)
				{
					const u32 index = m_free;

					WeakEntry* e = entry(index);
					m_free       = e->next_free;

					e->index     = index;
					e->weak_refs = 0;
					e->ptr       = object;

					return index;
				}

				const u32 index = m_size++;

				if ((index % page_size) == 0)
					m_pages.push_back(trx_new Page);

				WeakEntry* e = entry(index);

				e->index     = index;
				e->weak_refs = 0;
				e->ptr       = object;

				return index;
			}

			void free(WeakEntry* e)
			{
				const u32 index = e->index;

				e->ptr       = nullptr;
				e->weak_refs = 0;

				e->next_free = m_free;
				m_free       = index;
			}

			void free(u32 index)
			{
				WeakEntry* e = entry(index);
				trinex_assert(e->index == index);
				free(e);
			}
		};

	}// namespace

	void RefCounted::add_ref() noexcept
	{
		const u32 previous = AtomicRef(m_ref_count).fetch_add(1, etl::memory_order_relaxed);
		trinex_assert_msg(previous > 0, "add_ref() called on dead object");
	}

	void RefCounted::release() noexcept
	{
		const u32 previous = AtomicRef(m_ref_count).fetch_sub(1, etl::memory_order_release);

		trinex_assert_msg(previous > 0, "release() called with zero references");

		if (previous == 1)
		{
			etl::atomic_thread_fence(etl::memory_order_acquire);
			release_weak_entry();
			destroy();
		}
	}

	u32 RefCounted::references() noexcept
	{
		return AtomicRef(m_ref_count).load(etl::memory_order_relaxed);
	}

	WeakEntry* RefCounted::weak_entry(RefCounted* object) noexcept
	{
		if (object == nullptr)
			return nullptr;

		WeakStorage* storage = WeakStorage::instance();
		ScopeLock lock(storage->lock());

		if (object->m_weak_index == WeakStorage::bad_link)
			object->m_weak_index = storage->allocate(object);

		return storage->entry(object->m_weak_index);
	}

	void RefCounted::add_weak_ref(WeakEntry* entry) noexcept
	{
		if (entry)
		{
			AtomicRef(entry->weak_refs).fetch_add(1, etl::memory_order_relaxed);
		}
	}

	void RefCounted::release_weak_ref(WeakEntry* entry) noexcept
	{
		if (entry == nullptr)
			return;

		WeakStorage* storage = WeakStorage::instance();
		ScopeLock lock(storage->lock());

		const u32 previous = AtomicRef(entry->weak_refs).fetch_sub(1, etl::memory_order_release);
		trinex_assert_msg(previous > 0, "WeakRef released with zero references");

		if (previous == 1 && entry->ptr == nullptr)
		{
			etl::atomic_thread_fence(etl::memory_order_acquire);
			storage->free(entry);
		}
	}

	RefCounted* RefCounted::weak_ptr(WeakEntry* entry) noexcept
	{
		if (entry == nullptr)
			return nullptr;

		WeakStorage* storage = WeakStorage::instance();
		ScopeLock lock(storage->lock());

		return entry->ptr;
	}

	RefCounted* RefCounted::lock_weak(WeakEntry* entry) noexcept
	{
		if (entry == nullptr)
			return nullptr;

		WeakStorage* storage = WeakStorage::instance();
		ScopeLock lock(storage->lock());

		RefCounted* ptr = entry->ptr;

		if (ptr == nullptr)
			return nullptr;

		AtomicRef ref(ptr->m_ref_count);
		u32 references = ref.load(etl::memory_order_relaxed);

		while (references > 0)
		{
			if (ref.compare_exchange_weak(references, references + 1, etl::memory_order_acquire, etl::memory_order_relaxed))
			{
				return ptr;
			}
		}

		return nullptr;
	}

	void RefCounted::release_weak_entry() noexcept
	{
		if (m_weak_index == WeakStorage::bad_link)
			return;

		WeakStorage* storage = WeakStorage::instance();
		ScopeLock lock(storage->lock());

		WeakEntry* entry = storage->entry(m_weak_index);
		trinex_assert(entry->ptr == this);

		entry->ptr   = nullptr;
		m_weak_index = WeakStorage::bad_link;

		if (AtomicRef(entry->weak_refs).load(etl::memory_order_acquire) == 0)
			storage->free(entry);
	}
}// namespace Trinex
