module;
#include <deque>

export module Trinex.Core.ETL.Deque;

import Trinex.Core.Memory.Heap;

namespace Trinex
{
	template<typename Type, typename AllocatorType = HeapAllocator::Adapter<Type>>
	using Deque = std::deque<Type, AllocatorType>;

	template<typename Type, typename AllocatorType, typename Ar>
	inline void serialize(Ar& ar, Deque<Type, AllocatorType>& list)
	{
		ar.serialize_container(list);
	}
} // namespace Trinex
