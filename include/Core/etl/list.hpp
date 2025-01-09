#pragma once
#include <Core/etl/allocator.hpp>
#include <Core/etl/archive_predef.hpp>
#include <forward_list>
#include <list>

namespace Engine
{
	class Archive;

	template<typename Type>
	using List = std::list<Type, Allocator<Type>>;

	template<typename Type>
	using ForwardList = std::forward_list<Type, Allocator<Type>>;

	template<typename Type, typename ArchiveType>
	inline bool trinex_serialize_list(ArchiveType& ar, List<Type>& list)
		requires(is_complete_archive_type<ArchiveType>)
	{
		return ar.serialize_container(list);
	}

	template<typename Type, typename ArchiveType>
	inline bool trinex_serialize_list(ArchiveType& ar, ForwardList<Type>& list)
		requires(is_complete_archive_type<ArchiveType>)
	{
		return ar.serialize_container(list);
	}

	template<typename T>
	struct Serializer<ForwardList<T>> {
		bool serialize(Archive& ar, ForwardList<T>& list)
		{
			return trinex_serialize_list(ar, list);
		}
	};

	template<typename T>
	struct Serializer<List<T>> {
		bool serialize(Archive& ar, List<T>& list)
		{
			return trinex_serialize_list(ar, list);
		}
	};
}// namespace Engine
