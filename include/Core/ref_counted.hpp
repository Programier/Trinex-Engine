#pragma once
#include <Core/etl/concepts.hpp>

namespace Trinex
{
	struct WeakEntry;

	template<typename>
	class WeakRef;

	class ENGINE_EXPORT RefCounted
	{
	private:
		u32 m_ref_count  = 1;
		u32 m_weak_index = 0xFFFFFFFF;

	private:
		static WeakEntry* weak_entry(RefCounted* object) noexcept;
		static void add_weak_ref(WeakEntry* entry) noexcept;
		static void release_weak_ref(WeakEntry* entry) noexcept;
		static RefCounted* weak_ptr(WeakEntry* entry) noexcept;
		static RefCounted* lock_weak(WeakEntry* entry) noexcept;

		void release_weak_entry() noexcept;

	protected:
		RefCounted() noexcept = default;

		virtual ~RefCounted() = default;
		virtual void destroy() noexcept { trx_delete_inline(this); }

	public:
		RefCounted(const RefCounted&)            = delete;
		RefCounted& operator=(const RefCounted&) = delete;

		RefCounted(RefCounted&&)            = delete;
		RefCounted& operator=(RefCounted&&) = delete;

		void add_ref() noexcept;
		void release() noexcept;
		u32 references() noexcept;

		template<typename>
		friend class Ref;

		template<typename>
		friend class WeakRef;

		friend struct ByteAllocatorDeleter;
	};

	template<typename T>
	class Ref
	{
	private:
		T* m_ptr = nullptr;

		struct AdoptTag {
		};

		explicit Ref(T* ptr, AdoptTag) noexcept : m_ptr(ptr) {}

		void add_ref() noexcept
		{
			if (m_ptr)
			{
				m_ptr->add_ref();
			}
		}

		void release() noexcept
		{
			if (m_ptr)
			{
				m_ptr->release();
			}
		}

	public:
		Ref() noexcept = default;

		Ref(null) noexcept {}

		Ref(const Ref& other) noexcept : m_ptr(other.m_ptr) { add_ref(); }
		Ref(Ref&& other) noexcept : m_ptr(other.m_ptr) { other.m_ptr = nullptr; }

		template<typename U>
		    requires etl::convertible_to<U*, T*>
		Ref(const Ref<U>& other) noexcept : m_ptr(other.m_ptr)
		{
			add_ref();
		}

		template<typename U>
		    requires etl::convertible_to<U*, T*>
		Ref(Ref<U>&& other) noexcept : m_ptr(other.m_ptr)
		{
			other.m_ptr = nullptr;
		}

		template<typename U>
		    requires etl::convertible_to<U*, T*>
		Ref(const WeakRef<U>& other) noexcept : Ref(other.lock())
		{}

		~Ref() { release(); }

		Ref& operator=(const Ref& other) noexcept
		{
			if (this != &other)
			{
				Ref tmp(other);
				swap(tmp);
			}

			return *this;
		}

		Ref& operator=(Ref&& other) noexcept
		{
			if (this != &other)
			{
				release();

				m_ptr       = other.m_ptr;
				other.m_ptr = nullptr;
			}

			return *this;
		}

		template<typename U>
		    requires etl::convertible_to<U*, T*>
		Ref& operator=(const WeakRef<U>& other) noexcept
		{
			Ref(other).swap(*this);
			return *this;
		}

		Ref& operator=(null) noexcept
		{
			reset();
			return *this;
		}

		void reset() noexcept { Ref{}.swap(*this); }

		void reset(T* ptr) noexcept { Ref::retain(ptr).swap(*this); }

		void swap(Ref& other) noexcept
		{
			T* tmp      = m_ptr;
			m_ptr       = other.m_ptr;
			other.m_ptr = tmp;
		}

		[[nodiscard]]
		T* value() const noexcept
		{
			return m_ptr;
		}

		[[nodiscard]]
		T* operator->() const noexcept
		{
			trinex_assert(m_ptr);
			return m_ptr;
		}

		[[nodiscard]]
		T& operator*() const noexcept
		{
			trinex_assert(m_ptr);
			return *m_ptr;
		}

		[[nodiscard]]
		explicit operator bool() const noexcept
		{
			return m_ptr != nullptr;
		}

		[[nodiscard]]
		bool operator==(const Ref& other) const noexcept
		{
			return m_ptr == other.m_ptr;
		}

		[[nodiscard]]
		bool operator==(null) const noexcept
		{
			return m_ptr == nullptr;
		}

		[[nodiscard]]
		static Ref adopt(T* ptr) noexcept
		{
			return Ref(ptr, AdoptTag{});
		}

		[[nodiscard]]
		static Ref retain(T* ptr) noexcept
		{
			if (ptr)
			{
				ptr->add_ref();
			}

			return Ref(ptr, AdoptTag{});
		}

		template<typename... Args>
		[[nodiscard]]
		static Ref make(Args&&... args)
		{
			return adopt(trx_new T(args...));
		}
	};

	template<typename T>
	class WeakRef
	{
	private:
		WeakEntry* m_entry = nullptr;

		void add_ref() noexcept { RefCounted::add_weak_ref(m_entry); }
		void release() noexcept { RefCounted::release_weak_ref(m_entry); }

	public:
		WeakRef() noexcept = default;

		WeakRef(null) noexcept {}

		WeakRef(T* ptr) noexcept : m_entry(RefCounted::weak_entry(ptr)) { add_ref(); }

		template<typename U>
		    requires etl::convertible_to<U*, T*>
		WeakRef(const Ref<U>& ref) noexcept : WeakRef(ref.value())
		{}

		WeakRef(const WeakRef& other) noexcept : m_entry(other.m_entry) { add_ref(); }
		WeakRef(WeakRef&& other) noexcept : m_entry(other.m_entry) { other.m_entry = nullptr; }

		template<typename U>
		    requires etl::convertible_to<U*, T*>
		WeakRef(const WeakRef<U>& other) noexcept : m_entry(other.m_entry)
		{
			add_ref();
		}

		template<typename U>
		    requires etl::convertible_to<U*, T*>
		WeakRef(WeakRef<U>&& other) noexcept : m_entry(other.m_entry)
		{
			other.m_entry = nullptr;
		}

		~WeakRef() { release(); }

		WeakRef& operator=(const WeakRef& other) noexcept
		{
			if (this != &other)
			{
				WeakRef tmp(other);
				swap(tmp);
			}

			return *this;
		}

		WeakRef& operator=(WeakRef&& other) noexcept
		{
			if (this != &other)
			{
				release();

				m_entry       = other.m_entry;
				other.m_entry = nullptr;
			}

			return *this;
		}

		template<typename U>
		    requires etl::convertible_to<U*, T*>
		WeakRef& operator=(const WeakRef<U>& other) noexcept
		{
			WeakRef(other).swap(*this);
			return *this;
		}

		template<typename U>
		    requires etl::convertible_to<U*, T*>
		WeakRef& operator=(WeakRef<U>&& other) noexcept
		{
			WeakRef tmp;
			tmp.m_entry   = other.m_entry;
			other.m_entry = nullptr;
			tmp.swap(*this);
			return *this;
		}

		WeakRef& operator=(T* ptr) noexcept
		{
			WeakRef(ptr).swap(*this);
			return *this;
		}

		template<typename U>
		    requires etl::convertible_to<U*, T*>
		WeakRef& operator=(const Ref<U>& ref) noexcept
		{
			WeakRef(ref).swap(*this);
			return *this;
		}

		WeakRef& operator=(null) noexcept
		{
			reset();
			return *this;
		}

		void reset() noexcept { WeakRef{}.swap(*this); }

		void reset(T* ptr) noexcept { WeakRef(ptr).swap(*this); }

		void swap(WeakRef& other) noexcept
		{
			WeakEntry* tmp = m_entry;
			m_entry        = other.m_entry;
			other.m_entry  = tmp;
		}

		[[nodiscard]]
		T* value() const noexcept
		{
			return static_cast<T*>(RefCounted::weak_ptr(m_entry));
		}

		[[nodiscard]]
		Ref<T> lock() const noexcept
		{
			return Ref<T>::adopt(static_cast<T*>(RefCounted::lock_weak(m_entry)));
		}

		[[nodiscard]]
		bool expired() const noexcept
		{
			return value() == nullptr;
		}

		[[nodiscard]]
		explicit operator bool() const noexcept
		{
			return value() != nullptr;
		}

		[[nodiscard]]
		bool operator==(const WeakRef& other) const noexcept
		{
			return m_entry == other.m_entry;
		}

		[[nodiscard]]
		bool operator==(null) const noexcept
		{
			return value() == nullptr;
		}

		template<typename>
		friend class WeakRef;
	};
}// namespace Trinex
