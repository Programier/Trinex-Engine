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

	template<typename Key, typename Value, typename HashType = Hash<Key>, typename Pred = std::equal_to<Key>>
	using Map = std::unordered_map<Key, Value, HashType, Pred, Allocator<Pair<const Key, Value>>>;

	template<typename Key, typename Value, typename HashType = Hash<Key>, typename Pred = std::equal_to<Key>>
	using MultiMap = std::unordered_multimap<Key, Value, HashType, Pred, Allocator<Pair<const Key, Value>>>;

	template<typename Key, typename Value, typename Compare = std::less<Key>>
	using TreeMap = std::map<Key, Value, Compare, Allocator<Pair<const Key, Value>>>;

	template<typename Key, typename Value, typename Compare = std::less<Key>>
	using TreeMultiMap = std::multimap<Key, Value, Compare, Allocator<Pair<const Key, Value>>>;


	template<typename Key, typename Value, typename HashType = Hash<Key>, typename Pred = std::equal_to<Key>,
	         typename ArchiveType>
	inline bool trinex_serialize_map(ArchiveType& ar, Map<Key, Value, HashType, Pred>& map)
	    requires(is_complete_archive_type<ArchiveType>)
	{
		return ar.serialize_map(map);
	}

	template<typename Key, typename Value, typename Compare = std::less<Key>, typename ArchiveType>
	inline bool trinex_serialize_map(ArchiveType& ar, TreeMap<Key, Value, Compare>& map)
	    requires(is_complete_archive_type<ArchiveType>)
	{
		return ar.serialize_map(map);
	}

	template<typename Key, typename Value, typename HashType, typename Pred>
	struct Serializer<Map<Key, Value, HashType, Pred>> {
		bool serialize(Archive& ar, Map<Key, Value, HashType, Pred>& map) { return trinex_serialize_map(ar, map); }
	};

	template<typename Key, typename Value, typename Compare>
	struct Serializer<TreeMap<Key, Value, Compare>> {
		bool serialize(Archive& ar, TreeMap<Key, Value, Compare>& map) { return trinex_serialize_map(ar, map); }
	};
}// namespace Engine
