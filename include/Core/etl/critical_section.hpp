#pragma once
#include <mutex>

namespace Engine
{
	class CriticalSection final
	{
	private:
		std::mutex m_mutex;

	public:
		inline void lock() { m_mutex.lock(); }
		inline bool try_lock() { return m_mutex.try_lock(); }
		inline void unlock() { m_mutex.unlock(); }
	};

	class ScopeLock final
	{
	private:
		CriticalSection& m_critical_section;

	public:
		explicit inline ScopeLock(CriticalSection& criticalSection) : m_critical_section(criticalSection)
		{
			m_critical_section.lock();
		}

		ScopeLock(const ScopeLock&)            = delete;
		ScopeLock& operator=(const ScopeLock&) = delete;

		inline ~ScopeLock() { m_critical_section.unlock(); }
	};
}// namespace Engine
