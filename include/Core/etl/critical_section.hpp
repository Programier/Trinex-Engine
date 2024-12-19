#pragma once
#include <Core/etl/atomic.hpp>

namespace Engine
{
	class CriticalSection final
	{
	private:
		AtomicFlag m_flag = ATOMIC_FLAG_INIT;

	public:
		inline void lock()
		{
			while (m_flag.test_and_set())
			{
				m_flag.wait(true);
			}
		}

		inline void unlock()
		{
			m_flag.clear();
			m_flag.notify_one();
		}
	};

	class ScopeLock final
	{
	private:
		CriticalSection& m_criticalSection;

	public:
		explicit inline ScopeLock(CriticalSection& criticalSection) : m_criticalSection(criticalSection)
		{
			m_criticalSection.lock();
		}

		ScopeLock(const ScopeLock&)            = delete;
		ScopeLock& operator=(const ScopeLock&) = delete;

		inline ~ScopeLock()
		{
			m_criticalSection.unlock();
		}
	};
}// namespace Engine
