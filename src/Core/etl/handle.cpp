#include <Core/etl/atomic.hpp>
#include <Core/etl/handle.hpp>

namespace Trinex
{
	u64 HandleBase::generate()
	{
		static Atomic<u64> next = 1;
		const u64 value         = next.fetch_add(1, etl::memory_order_relaxed);

		trinex_verify_msg(value != 0, "Handle ID overflow: exhausted all available 64-bit handle values");

		return value;
	}
}// namespace Trinex
