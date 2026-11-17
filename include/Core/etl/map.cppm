module;
#include <Core/etl/std.hpp>
export module Trinex.Core.ETL:Map;
export import Trinex.Core.Memory.Heap;
export import :Functional;
export import :Utility;

export namespace Trinex
{
	template<typename Key, typename Value, typename HashType = Hash<Key>, typename Pred = EqualTo<Key>,
	         typename AllocatorType = HeapAllocator::Adapter<Pair<const Key, Value>>>
	using Map = std::unordered_map<Key, Value, HashType, Pred, AllocatorType>;

	template<typename Key, typename Value, typename HashType = Hash<Key>, typename Pred = std::equal_to<Key>,
	         typename AllocatorType = HeapAllocator::Adapter<Pair<const Key, Value>>>
	using MultiMap = std::unordered_multimap<Key, Value, HashType, Pred, AllocatorType>;

	template<typename Key, typename Value, typename Compare = Less<Key>,
	         typename AllocatorType = HeapAllocator::Adapter<Pair<const Key, Value>>>
	using TreeMap = std::map<Key, Value, Compare, AllocatorType>;

	template<typename Key, typename Value, typename Compare = Less<Key>,
	         typename AllocatorType = HeapAllocator::Adapter<Pair<const Key, Value>>>
	using TreeMultiMap = std::multimap<Key, Value, Compare, AllocatorType>;

	template<typename Key, typename Value, typename HashType, typename Pred, typename AllocatorType, typename Ar>
	inline bool serialize(Ar& ar, Map<Key, Value, HashType, Pred, AllocatorType>& map)
	{
		return ar.serialize_map(map);
	}

	template<typename Key, typename Value, typename Compare = std::less<Key>, typename AllocatorType, typename Ar>
	inline void serialize(Ar& ar, TreeMap<Key, Value, Compare, AllocatorType>& map)
	{
		return ar.serialize_map(map);
	}
} // namespace Trinex
