#pragma once
#include <atomic>

namespace Engine
{
	template<typename Type>
	using Atomic = std::atomic<Type>;

	using AtomicFlag = std::atomic_flag;

	namespace etl
	{
		inline constexpr auto memory_order_relaxed = std::memory_order::relaxed;
		inline constexpr auto memory_order_consume = std::memory_order::consume;
		inline constexpr auto memory_order_acquire = std::memory_order::acquire;
		inline constexpr auto memory_order_release = std::memory_order::release;
		inline constexpr auto memory_order_acq_rel = std::memory_order::acq_rel;
		inline constexpr auto memory_order_seq_cst = std::memory_order::seq_cst;
	}// namespace etl
}// namespace Engine
