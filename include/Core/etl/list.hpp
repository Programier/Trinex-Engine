#pragma once
#include <Core/etl/allocator.hpp>
#include <Core/etl/archive_predef.hpp>
#include <forward_list>
#include <list>

namespace Engine
{
	class Archive;

	template<typename Type, typename AllocatorType = Allocator<Type>>
	using List = std::list<Type, AllocatorType>;

	template<typename Type, typename AllocatorType = Allocator<Type>>
	using ForwardList = std::forward_list<Type, AllocatorType>;

	template<typename Type, typename AllocatorType, typename ArchiveType>
	inline bool trinex_serialize_list(ArchiveType& ar, List<Type, AllocatorType>& list)
	    requires(is_complete_archive_type<ArchiveType>)
	{
		return ar.serialize_container(list);
	}

	template<typename Type, typename AllocatorType, typename ArchiveType>
	inline bool trinex_serialize_list(ArchiveType& ar, ForwardList<Type, AllocatorType>& list)
	    requires(is_complete_archive_type<ArchiveType>)
	{
		return ar.serialize_container(list);
	}

	template<typename T, typename AllocatorType>
	struct Serializer<ForwardList<T, AllocatorType>> {
		bool serialize(Archive& ar, ForwardList<T, AllocatorType>& list) { return trinex_serialize_list(ar, list); }
	};

	template<typename T, typename AllocatorType>
	struct Serializer<List<T, AllocatorType>> {
		bool serialize(Archive& ar, List<T, AllocatorType>& list) { return trinex_serialize_list(ar, list); }
	};
}// namespace Engine
