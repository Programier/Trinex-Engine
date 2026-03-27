#pragma once
#include <mutex>

namespace Trinex
{
	using CriticalSection          = std::mutex;
	using CriticalSectionRecursive = std::recursive_mutex;

	template<typename T>
	class ScopeLock final
	{
	private:
		T& m_critical_section;

	public:
		explicit inline ScopeLock(T& section) : m_critical_section(section) { m_critical_section.lock(); }

		ScopeLock(const ScopeLock&)            = delete;
		ScopeLock& operator=(const ScopeLock&) = delete;

		inline ~ScopeLock() { m_critical_section.unlock(); }
	};
}// namespace Trinex
