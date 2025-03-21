#pragma once
#include <Core/export.hpp>

namespace Engine
{
	class ENGINE_EXPORT RenderResourcePtrBase
	{
	protected:
		static void release(void* object);
	};

	template<typename T>
	class RenderResourcePtr final : private RenderResourcePtrBase
	{
	private:
		T* m_ptr = nullptr;

	public:
		RenderResourcePtr() = default;
		explicit RenderResourcePtr(T* ptr) : m_ptr(ptr) {}
		~RenderResourcePtr() { reset(); }

		RenderResourcePtr(const RenderResourcePtr&)            = delete;
		RenderResourcePtr& operator=(const RenderResourcePtr&) = delete;

		RenderResourcePtr(RenderResourcePtr&& other) noexcept
		{
			m_ptr       = other.m_ptr;
			other.m_ptr = nullptr;
		}

		RenderResourcePtr& operator=(RenderResourcePtr&& other) noexcept
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

		RenderResourcePtr& reset(T* new_ptr = nullptr)
		{
			if (m_ptr)
			{
				RenderResourcePtrBase::release(m_ptr);
			}
			m_ptr = new_ptr;
			return *this;
		}

		RenderResourcePtr& swap(RenderResourcePtr& other) noexcept
		{
			T* tmp      = m_ptr;
			m_ptr       = other.m_ptr;
			other.m_ptr = tmp;
			return *this;
		}

		RenderResourcePtr& operator=(T* ptr) { return reset(ptr); }
	};
}// namespace Engine
