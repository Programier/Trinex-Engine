#pragma once
#include <Core/etl/allocator.hpp>
#include <Core/etl/archive_predef.hpp>
#include <Core/etl/hash.hpp>
#include <set>
#include <unordered_set>

namespace Engine
{
	class Archive;

	template<typename Type, typename HashType = Hash<Type>, typename Pred = std::equal_to<Type>,
	         typename AllocatorType = Allocator<Type>>
	using Set = std::unordered_set<Type, HashType, Pred, AllocatorType>;

	template<typename Type, typename HashType = Hash<Type>, typename Pred = std::equal_to<Type>,
	         typename AllocatorType = Allocator<Type>>
	using MultiSet = std::unordered_multiset<Type, HashType, Pred, AllocatorType>;

	template<typename Type, typename Compare = std::less<Type>, typename AllocatorType = Allocator<Type>>
	using TreeSet = std::set<Type, Compare, AllocatorType>;

	template<typename Type, typename Compare = std::less<Type>, typename AllocatorType = Allocator<Type>>
	using TreeMultiSet = std::multiset<Type, Compare, AllocatorType>;

	template<typename Type, typename HashType = Hash<Type>, typename Pred = std::equal_to<Type>,
	         typename AllocatorType = Allocator<Type>, typename ArchiveType>
	inline bool trinex_serialize_set(ArchiveType& ar, Set<Type, HashType, Pred, AllocatorType>& set)
	    requires(is_complete_archive_type<ArchiveType>)
	{
		return ar.process_set(set);
	}

	template<typename Type, typename Compare, typename AllocatorType, typename ArchiveType>
	inline bool trinex_serialize_set(ArchiveType& ar, TreeSet<Type, Compare, AllocatorType>& set)
	    requires(is_complete_archive_type<ArchiveType>)
	{
		return ar.process_set(set);
	}

	template<typename Type, typename HashType, typename Pred, typename AllocatorType>
	struct Serializer<Set<Type, HashType, Pred, AllocatorType>> {
		bool serialize(Archive& ar, Set<Type, HashType, Pred, AllocatorType>& set) { return trinex_serialize_set(set); }
	};

	template<typename Type, typename Compare, typename AllocatorType>
	struct Serializer<TreeSet<Type, Compare, AllocatorType>> {
		bool serialize(Archive& ar, TreeSet<Type, Compare, AllocatorType>& set) { return trinex_serialize_set(set); }
	};
}// namespace Engine
