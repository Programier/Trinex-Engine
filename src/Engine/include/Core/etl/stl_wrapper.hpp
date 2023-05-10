#pragma once
#include <array>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <vector>


namespace Engine
{
    template<typename Type>
    using Allocator = std::allocator<Type>;

    template<typename Type>
    using Hash = std::hash<Type>;

    template<typename T1, typename T2>
    using Pair = std::pair<T1, T2>;

    template<typename Type, size_t size>
    using Array = std::array<Type, size>;

    template<typename Type, typename AllocatorType = Allocator<Type>>
    using Vector = std::vector<Type, AllocatorType>;

    template<typename Type, typename AllocatorType = Allocator<Type>>
    using List = std::list<Type, AllocatorType>;

    template<typename Type, typename AllocatorType = Allocator<Type>>
    using ForwardList = std::forward_list<Type, AllocatorType>;

    template<typename Type, typename HashType = Hash<Type>, typename Pred = std::equal_to<Type>,
             typename AllocatorType = Allocator<Type>>
    using Set = std::unordered_set<Type, HashType, Pred, AllocatorType>;

    template<typename Key, typename Value, typename HashType = Hash<Key>, typename Pred = std::equal_to<Key>,
             typename AllocatorType = Allocator<Pair<const Key, Value>>>
    using Map = std::unordered_map<Key, Value, HashType, Pred, AllocatorType>;

    template<typename Type, size_t extend = std::dynamic_extent>
    using Span = std::span<Type, extend>;

    template<typename Type, typename Compare = std::less<Type>, typename AllocatorType = Allocator<Type>>
    using TreeSet = std::set<Type, Compare, AllocatorType>;

    template<typename Key, typename Value, typename Compare = std::less<Key>,
             typename AllocatorType = Allocator<Pair<const Key, Value>>>
    using TreeMap = std::map<Key, Value, Compare, AllocatorType>;


    template<size_t size>
    using BitSet = std::bitset<size>;
}// namespace Engine
