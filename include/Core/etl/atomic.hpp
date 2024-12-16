#pragma once
#include <atomic>

namespace Engine
{
	template<typename Type>
	using Atomic = std::atomic<Type>;

	using AtomicFlag = std::atomic_flag;
}// namespace Engine
