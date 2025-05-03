#pragma once
#include <Core/etl/allocator.hpp>
#include <Core/etl/archive_predef.hpp>
#include <Core/etl/hash.hpp>
#include <Core/etl/pair.hpp>
#include <map>
#include <unordered_map>

namespace Engine
{
	class Archive;

	template<typename Key, typename Value, typename HashType = Hash<Key>, typename Pred = std::equal_to<Key>,
	         typename AllocatorType = Allocator<Pair<const Key, Value>>>
	using Map = std::unordered_map<Key, Value, HashType, Pred, AllocatorType>;

	template<typename Key, typename Value, typename HashType = Hash<Key>, typename Pred = std::equal_to<Key>,
	         typename AllocatorType = Allocator<Pair<const Key, Value>>>
	using MultiMap = std::unordered_multimap<Key, Value, HashType, Pred, AllocatorType>;

	template<typename Key, typename Value, typename Compare = std::less<Key>,
	         typename AllocatorType = Allocator<Pair<const Key, Value>>>
	using TreeMap = std::map<Key, Value, Compare, AllocatorType>;

	template<typename Key, typename Value, typename Compare = std::less<Key>,
	         typename AllocatorType = Allocator<Pair<const Key, Value>>>
	using TreeMultiMap = std::multimap<Key, Value, Compare, AllocatorType>;

	template<typename Key, typename Value, typename HashType = Hash<Key>, typename Pred = std::equal_to<Key>,
	         typename AllocatorType, typename ArchiveType>
	inline bool trinex_serialize_map(ArchiveType& ar, Map<Key, Value, HashType, Pred, AllocatorType>& map)
	    requires(is_complete_archive_type<ArchiveType>)
	{
		return ar.serialize_map(map);
	}

	template<typename Key, typename Value, typename Compare = std::less<Key>, typename AllocatorType, typename ArchiveType>
	inline bool trinex_serialize_map(ArchiveType& ar, TreeMap<Key, Value, Compare, AllocatorType>& map)
	    requires(is_complete_archive_type<ArchiveType>)
	{
		return ar.serialize_map(map);
	}

	template<typename Key, typename Value, typename HashType, typename Pred, typename AllocatorType>
	struct Serializer<Map<Key, Value, HashType, Pred, AllocatorType>> {
		bool serialize(Archive& ar, Map<Key, Value, HashType, Pred, AllocatorType>& map) { return trinex_serialize_map(ar, map); }
	};

	template<typename Key, typename Value, typename Compare, typename AllocatorType>
	struct Serializer<TreeMap<Key, Value, Compare, AllocatorType>> {
		bool serialize(Archive& ar, TreeMap<Key, Value, Compare, AllocatorType>& map) { return trinex_serialize_map(ar, map); }
	};
}// namespace Engine
