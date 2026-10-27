#pragma once

namespace Trinex
{
	class RefCounted;

	template<typename>
	class WeakRef;

	class RefBase
	{
	protected:
		void add_ref(void* object);
		void release(void* object);
	};

	class WeakRefBase
	{
	protected:
		static void* entry(RefCounted* object) noexcept;
		static void add_ref(void* entry) noexcept;
		static void release(void* entry) noexcept;
		static RefCounted* weak_ptr(void* entry) noexcept;
		static RefCounted* lock_weak(void* entry) noexcept;

	public:
	};

	template<typename T>
	class Ref : private RefBase
	{
	private:
		T* m_ptr = nullptr;

		struct AdoptTag
		{
		};

		explicit Ref(T* ptr, AdoptTag) noexcept : m_ptr(ptr) {}

		void add_ref() noexcept
		{
			if (m_ptr)
			{
				RefBase::add_ref(m_ptr);
			}
		}

		void release() noexcept
		{
			if (m_ptr)
			{
				RefBase::release(m_ptr);
			}
		}

	public:
		Ref() noexcept = default;
		Ref(null) noexcept {}

		Ref(const Ref& other) noexcept : m_ptr(other.m_ptr) { add_ref(); }
		Ref(Ref&& other) noexcept : m_ptr(other.m_ptr) { other.m_ptr = nullptr; }

		template<typename U>
		    requires(trinex_is_base_of(T, U))
		Ref(const Ref<U>& other) noexcept : m_ptr(static_cast<T*>(other.m_ptr))
		{
			add_ref();
		}

		template<typename U>
		    requires(trinex_is_base_of(T, U))
		Ref(Ref<U>&& other) noexcept : m_ptr(static_cast<T*>(other.m_ptr))
		{
			other.m_ptr = nullptr;
		}

		template<typename U>
		    requires(trinex_is_base_of(T, U))
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
		    requires(trinex_is_base_of(T, U))
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
		T* detach() noexcept
		{
			T* tmp = m_ptr;
			m_ptr  = nullptr;
			return tmp;
		}

		[[nodiscard]]
		T* operator->() const noexcept
		{
			T* ptr = value();
			trinex_assert(ptr);
			return ptr;
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
		bool operator==(const T* ptr) const noexcept
		{
			return m_ptr == ptr;
		}

		[[nodiscard]]
		static Ref adopt(T* ptr) noexcept
		{
			return Ref(ptr, AdoptTag{});
		}

		[[nodiscard]]
		static Ref retain(T* ptr) noexcept
		{
			Ref result(ptr, AdoptTag{});
			result.add_ref();
			return result;
		}

		template<typename... Args>
		[[nodiscard]]
		static Ref make(Args&&... args)
		{
			return adopt(trx_new T(static_cast<Args&&>(args)...));
		}

		template<typename>
		friend class Ref;
	};

	template<typename T>
	class WeakRef : private WeakRefBase
	{
	private:
		void* m_entry = nullptr;

		void add_ref() noexcept { WeakRefBase::add_ref(m_entry); }
		void release() noexcept { WeakRefBase::release(m_entry); }

	public:
		WeakRef() noexcept = default;

		WeakRef(null) noexcept {}

		WeakRef(T* ptr) noexcept : m_entry(WeakRefBase::entry(static_cast<RefCounted*>(ptr))) { add_ref(); }

		template<typename U>
		    requires(trinex_is_base_of(T, U))
		WeakRef(const Ref<U>& ref) noexcept : WeakRef(ref.value())
		{}

		WeakRef(const WeakRef& other) noexcept : m_entry(other.m_entry) { add_ref(); }
		WeakRef(WeakRef&& other) noexcept : m_entry(other.m_entry) { other.m_entry = nullptr; }

		template<typename U>
		    requires(trinex_is_base_of(T, U))
		WeakRef(const WeakRef<U>& other) noexcept : m_entry(other.m_entry)
		{
			add_ref();
		}

		template<typename U>
		    requires(trinex_is_base_of(T, U))
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
		    requires(trinex_is_base_of(T, U))
		WeakRef& operator=(const WeakRef<U>& other) noexcept
		{
			WeakRef(other).swap(*this);
			return *this;
		}

		template<typename U>
		    requires(trinex_is_base_of(T, U))
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
		    requires(trinex_is_base_of(T, U))
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
			void* tmp     = m_entry;
			m_entry       = other.m_entry;
			other.m_entry = tmp;
		}

		[[nodiscard]]
		T* value() const noexcept
		{
			return static_cast<T*>(WeakRefBase::weak_ptr(m_entry));
		}

		[[nodiscard]]
		Ref<T> lock() const noexcept
		{
			return Ref<T>::adopt(static_cast<T*>(WeakRefBase::lock_weak(m_entry)));
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
		bool operator==(const T* ptr) const noexcept
		{
			return value() == ptr;
		}

		template<typename>
		friend class WeakRef;
	};

	template<typename T>
	[[nodiscard]]
	inline Ref<T> adopt_ref(T* ptr) noexcept
	{
		return Ref<T>::adopt(ptr);
	}

	template<typename T>
	[[nodiscard]]
	inline Ref<T> retain_ref(T* ptr) noexcept
	{
		return Ref<T>::retain(ptr);
	}
} // namespace Trinex
