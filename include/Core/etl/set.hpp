#pragma once
#include <Core/etl/allocator.hpp>
#include <Core/etl/archive_predef.hpp>
#include <Core/etl/hash.hpp>
#include <set>
#include <unordered_set>

namespace Engine
{
	class Archive;

	template<typename Type, typename HashType = Hash<Type>, typename Pred = std::equal_to<Type>>
	using Set = std::unordered_set<Type, HashType, Pred, Allocator<Type>>;

	template<typename Type, typename HashType = Hash<Type>, typename Pred = std::equal_to<Type>>
	using MultiSet = std::unordered_multiset<Type, HashType, Pred, Allocator<Type>>;

	template<typename Type, typename Compare = std::less<Type>>
	using TreeSet = std::set<Type, Compare, Allocator<Type>>;

	template<typename Type, typename Compare = std::less<Type>>
	using TreeMultiSet = std::multiset<Type, Compare, Allocator<Type>>;

	template<typename Type, typename HashType = Hash<Type>, typename Pred = std::equal_to<Type>, typename ArchiveType>
	inline bool trinex_serialize_set(ArchiveType& ar, Set<Type, HashType, Pred>& set)
	    requires(is_complete_archive_type<ArchiveType>)
	{
		return ar.process_set(set);
	}

	template<typename Type, typename Compare = std::less<Type>, typename ArchiveType>
	inline bool trinex_serialize_set(ArchiveType& ar, TreeSet<Type, Compare>& set)
	    requires(is_complete_archive_type<ArchiveType>)
	{
		return ar.process_set(set);
	}

	template<typename Type, typename HashType, typename Pred>
	struct Serializer<Set<Type, HashType, Pred>> {
		bool serialize(Archive& ar, Set<Type, HashType, Pred>& set) { return trinex_serialize_set(set); }
	};

	template<typename Type, typename Compare>
	struct Serializer<TreeSet<Type, Compare>> {
		bool serialize(Archive& ar, TreeSet<Type, Compare>& set) { return trinex_serialize_set(set); }
	};
}// namespace Engine
