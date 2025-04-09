#include <Core/tickable.hpp>

namespace Engine
{
	TickableObject* TickableObject::s_first = nullptr;
	TickableObject* TickableObject::s_last  = nullptr;

	TickableObject::TickableObject()
	{
		if (s_last)
		{
			s_last->m_next = this;
			m_prev         = s_last;
			s_last         = this;
		}
		else
		{
			s_first = s_last = this;
		}
	}

	TickableObject& TickableObject::update(float dt)
	{
		return *this;
	}

	bool TickableObject::is_tickable() const
	{
		return true;
	}

	bool TickableObject::is_tickable_when_paused() const
	{
		return true;
	}

	TickableObject::~TickableObject()
	{
		if (m_prev)
		{
			m_prev->m_next = m_next;
		}
		else
		{
			s_first = m_next;
		}

		if (m_next)
		{
			m_next->m_prev = m_prev;
		}
		else
		{
			s_last = m_prev;
		}
	}
}// namespace Engine
