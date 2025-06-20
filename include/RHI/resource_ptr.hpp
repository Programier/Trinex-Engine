#pragma once
#include <Core/export.hpp>

namespace Engine
{

	class ENGINE_EXPORT RHIResourcePtrBase
	{
	protected:
		static void release(void* object);
	};

	template<typename T>
	class RHIResourcePtr final : private RHIResourcePtrBase
	{
	private:
		T* m_ptr = nullptr;

	public:
		RHIResourcePtr() = default;
		explicit RHIResourcePtr(T* ptr) : m_ptr(ptr) {}
		~RHIResourcePtr() { reset(); }

		RHIResourcePtr(const RHIResourcePtr&)            = delete;
		RHIResourcePtr& operator=(const RHIResourcePtr&) = delete;

		RHIResourcePtr(RHIResourcePtr&& other) noexcept
		{
			m_ptr       = other.m_ptr;
			other.m_ptr = nullptr;
		}

		RHIResourcePtr& operator=(RHIResourcePtr&& other) noexcept
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

		RHIResourcePtr& reset(T* new_ptr = nullptr)
		{
			if (m_ptr)
			{
				RHIResourcePtrBase::release(m_ptr);
			}
			m_ptr = new_ptr;
			return *this;
		}

		RHIResourcePtr& swap(RHIResourcePtr& other) noexcept
		{
			T* tmp      = m_ptr;
			m_ptr       = other.m_ptr;
			other.m_ptr = tmp;
			return *this;
		}

		RHIResourcePtr& operator=(T* ptr) { return reset(ptr); }
	};
}// namespace Engine
