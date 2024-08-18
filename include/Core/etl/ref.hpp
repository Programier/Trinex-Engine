#pragma once
#include <utility>

namespace Engine
{
	template<typename T>
	class LRef
	{
	private:
		T* m_ref;


	public:
		LRef(T& ptr) : m_ref(&ptr)
		{}

		LRef(const LRef& value) : m_ref(value.m_ref)
		{}

		LRef& operator=(const LRef& value)
		{
			if (this == &value)
				return *this;
			m_ref = value.m_ref;
			return *this;
		}

		T& get()
		{
			return *m_ref;
		}

		const T& get() const
		{
			return *m_ref;
		}


		T* address()
		{
			return m_ref;
		}

		const T* address() const
		{
			return m_ref;
		}

		friend struct Ref;
	};

	template<typename T>
	class RRef
	{
	private:
		T m_ref;


	public:
		RRef(T&& value) : m_ref(std::move(value))
		{}

		RRef(const RRef& from) = delete;

		RRef(RRef&& from) : m_ref(std::move(from.m_ref))
		{}

		RRef& operator=(const RRef& value) = delete;

		RRef& operator=(RRef&& value)
		{
			if (this == &value)
				return *this;

			m_ref = std::move(value.m_ref);
			return *this;
		}

		T&& get()
		{
			return std::move(m_ref);
		}

		const T&& get() const
		{
			return std::move(m_ref);
		}

		T* address()
		{
			return &m_ref;
		}

		const T* address() const
		{
			return &m_ref;
		}

		friend struct Ref;
	};
}// namespace Engine
