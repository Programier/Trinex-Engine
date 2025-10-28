#pragma once
#include <mutex>

namespace Engine
{
	using CriticalSection = std::mutex;

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
