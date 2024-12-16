#pragma once
#include <Core/etl/atomic.hpp>

namespace Engine
{
	class CriticalSection
	{
	private:
		AtomicFlag m_flag = ATOMIC_FLAG_INIT;

	public:
		void lock()
		{
			while (m_flag.test_and_set())
			{
				m_flag.wait(true);
			}
		}

		void unlock()
		{
			m_flag.clear();
			m_flag.notify_one();
		}
	};
}// namespace Engine
