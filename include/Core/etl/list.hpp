#pragma once
#include <Core/etl/allocator.hpp>
#include <forward_list>
#include <list>

namespace Engine
{
	class Archive;

	template<typename Type>
	using List = std::list<Type, Allocator<Type>>;

	template<typename Type>
	using ForwardList = std::forward_list<Type, Allocator<Type>>;

	template<typename Type, typename ArchiveType = Archive>
	inline bool operator&(ArchiveType& ar, List<Type>& list)
	{
#if TRINEX_ARCHIVE_INCLUDED
		return ar.write_container(list);
#else
		static_assert(false, "Archive class is incomplete. Please, include <Core/archive.hpp> first");
		return false;
#endif
	}

	template<typename Type, typename ArchiveType = Archive>
	inline bool operator&(ArchiveType& ar, ForwardList<Type>& list)
	{
#if TRINEX_ARCHIVE_INCLUDED
		return ar.write_container(list);
#else
		static_assert(false, "Archive class is incomplete. Please, include <Core/archive.hpp> first");
		return false;
#endif
	}
}// namespace Engine
