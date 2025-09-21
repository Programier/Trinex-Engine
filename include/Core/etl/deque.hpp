#pragma once
#include <Core/etl/allocator.hpp>
#include <Core/etl/archive_predef.hpp>
#include <deque>

namespace Engine
{
	template<typename Type, typename AllocatorType = Allocator<Type>>
	using Deque = std::deque<Type, AllocatorType>;

	template<typename Type, typename AllocatorType, typename ArchiveType>
	inline bool trinex_serialize_deque(ArchiveType& ar, Deque<Type, AllocatorType>& list)
	    requires(is_complete_archive_type<ArchiveType>)
	{
		return ar.serialize_container(list);
	}

	template<typename T, typename AllocatorType>
	struct Serializer<Deque<T, AllocatorType>> {
		bool serialize(Archive& ar, Deque<T, AllocatorType>& list) { return trinex_serialize_deque(ar, list); }
	};
}// namespace Engine
