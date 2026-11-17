module Trinex.Core.RefCounted;

import Trinex.Core.Assert;
import Trinex.Core.ETL;

namespace Trinex
{
	struct WeakEntry
	{
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
			struct Page
			{
				WeakEntry entries[page_size];
			};

		private:
			CriticalSection m_lock;
			Array<Page*> m_pages;
			u32 m_size = 0;
			u32 m_free = bad_link;

		public:
			CriticalSection& lock() { return m_lock; }

			static WeakStorage* instance()
			{
				static WeakStorage* storage = new WeakStorage;
				return storage;
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
					m_pages.push_back(new Page);

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
				Assert::assertion(e->index == index, "Invalid weak entry index");
				free(e);
			}
		};

	} // namespace

	void RefCounted::add_ref() const noexcept
	{
		const u32 previous = AtomicRef(m_ref_count).fetch_add(1, etl::memory_order_relaxed);
		Assert::assertion(previous > 0, "add_ref() called on dead object");
	}

	void RefCounted::release() const noexcept
	{
		const u32 previous = AtomicRef(m_ref_count).fetch_sub(1, etl::memory_order_release);

		Assert::assertion(previous > 0, "release() called with zero references");

		if (previous == 1)
		{
			etl::atomic_thread_fence(etl::memory_order_acquire);
			release_weak_entry();
			const_cast<RefCounted*>(this)->destroy();
		}
	}

	void RefCounted::revive() const noexcept
	{
		const u32 previous = AtomicRef(m_ref_count).fetch_add(1, etl::memory_order_relaxed);
		Assert::assertion(previous == 0, "revive() called on alive object");
	}

	RefCounted::~RefCounted() {}

	u32 RefCounted::references() const noexcept
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
		Assert::assertion(previous > 0, "WeakRef released with zero references");

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

	void RefCounted::release_weak_entry() const noexcept
	{
		if (m_weak_index == WeakStorage::bad_link)
			return;

		WeakStorage* storage = WeakStorage::instance();
		ScopeLock lock(storage->lock());

		WeakEntry* entry = storage->entry(m_weak_index);
		Assert::assertion(entry->ptr == this, "Invalid weak ptr");

		entry->ptr   = nullptr;
		m_weak_index = WeakStorage::bad_link;

		if (AtomicRef(entry->weak_refs).load(etl::memory_order_acquire) == 0)
			storage->free(entry);
	}

	void RefBase::add_ref(void* object)
	{
		static_cast<RefCounted*>(object)->add_ref();
	}

	void RefBase::release(void* object)
	{
		static_cast<RefCounted*>(object)->release();
	}

	void* WeakRefBase::entry(RefCounted* object) noexcept
	{
		return RefCounted::weak_entry(object);
	}

	void WeakRefBase::add_ref(void* entry) noexcept
	{
		RefCounted::add_weak_ref(static_cast<WeakEntry*>(entry));
	}

	void WeakRefBase::release(void* entry) noexcept
	{
		RefCounted::release_weak_ref(static_cast<WeakEntry*>(entry));
	}

	RefCounted* WeakRefBase::weak_ptr(void* entry) noexcept
	{
		return RefCounted::weak_ptr(static_cast<WeakEntry*>(entry));
	}

	RefCounted* WeakRefBase::lock_weak(void* entry) noexcept
	{
		return RefCounted::lock_weak(static_cast<WeakEntry*>(entry));
	}
} // namespace Trinex
