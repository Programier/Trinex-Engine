#pragma once

namespace Trinex::Platform
{
	class ENGINE_EXPORT ResourcePtrBase
	{
	protected:
		static void release(void* object);
	};

	template<typename T>
	class ResourcePtr final : private ResourcePtrBase
	{
	private:
		T* m_ptr = nullptr;

	public:
		ResourcePtr() = default;
		explicit ResourcePtr(T* ptr) : m_ptr(ptr) {}
		~ResourcePtr() { reset(); }

		ResourcePtr(const ResourcePtr&)            = delete;
		ResourcePtr& operator=(const ResourcePtr&) = delete;

		ResourcePtr(ResourcePtr&& other) noexcept
		{
			m_ptr       = other.m_ptr;
			other.m_ptr = nullptr;
		}

		ResourcePtr& operator=(ResourcePtr&& other) noexcept
		{
			if (this != &other)
			{
				reset();
				m_ptr       = other.m_ptr;
				other.m_ptr = nullptr;
			}
			return *this;
		}

		T* get() const { return m_ptr; }
		T* operator->() const { return m_ptr; }
		T& operator*() const { return *m_ptr; }
		operator T*() const { return m_ptr; }

		ResourcePtr& reset(T* new_ptr = nullptr)
		{
			if (m_ptr)
			{
				ResourcePtrBase::release(m_ptr);
			}
			m_ptr = new_ptr;
			return *this;
		}

		ResourcePtr& swap(ResourcePtr& other) noexcept
		{
			T* tmp      = m_ptr;
			m_ptr       = other.m_ptr;
			other.m_ptr = tmp;
			return *this;
		}

		ResourcePtr& operator=(T* ptr) { return reset(ptr); }
	};
}// namespace Trinex::Platform
