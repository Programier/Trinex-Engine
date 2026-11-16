module;

#include <forward_list>
#include <list>

export module Trinex.Core.ETL.List;
export import Trinex.Core.Memory.Heap;

export namespace Trinex
{
	class Archive;

	template<typename Type, typename AllocatorType = HeapAllocator::Adapter<Type>>
	using List = std::list<Type, AllocatorType>;

	template<typename Type, typename AllocatorType = HeapAllocator::Adapter<Type>>
	using ForwardList = std::forward_list<Type, AllocatorType>;

	template<typename Ar, typename Type, typename AllocatorType>
	void serialize(Ar& ar, List<Type, AllocatorType>& list)
	{
		ar.serialize_container(list);
	}

	template<typename Ar, typename Type, typename AllocatorType>
	void serialize(Ar& ar, ForwardList<Type, AllocatorType>& list)
	{
		ar.serialize_container(list);
	}
} // namespace Trinex
