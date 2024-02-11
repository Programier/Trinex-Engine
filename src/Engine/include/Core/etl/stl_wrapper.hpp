#pragma once
#include <Core/etl/any.hpp>
#include <array>
#include <bitset>
#include <forward_list>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
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

    template<typename Type, typename HashType = Hash<Type>, typename Pred = std::equal_to<Type>,
             typename AllocatorType = Allocator<Type>>
    using MultiSet = std::unordered_multiset<Type, HashType, Pred, AllocatorType>;

    template<typename Key, typename Value, typename HashType = Hash<Key>, typename Pred = std::equal_to<Key>,
             typename AllocatorType = Allocator<Pair<const Key, Value>>>
    using Map = std::unordered_map<Key, Value, HashType, Pred, AllocatorType>;

    template<typename Key, typename Value, typename HashType = Hash<Key>, typename Pred = std::equal_to<Key>,
             typename AllocatorType = Allocator<Pair<const Key, Value>>>
    using MultiMap = std::unordered_multimap<Key, Value, HashType, Pred, AllocatorType>;

    template<typename Type, size_t extend = std::dynamic_extent>
    using Span = std::span<Type, extend>;

    template<typename Type, typename Compare = std::less<Type>, typename AllocatorType = Allocator<Type>>
    using TreeSet = std::set<Type, Compare, AllocatorType>;

    template<typename Type, typename Compare = std::less<Type>, typename AllocatorType = Allocator<Type>>
    using TreeMultiSet = std::multiset<Type, Compare, AllocatorType>;

    template<typename Key, typename Value, typename Compare = std::less<Key>,
             typename AllocatorType = Allocator<Pair<const Key, Value>>>
    using TreeMap = std::map<Key, Value, Compare, AllocatorType>;

    template<typename Key, typename Value, typename Compare = std::less<Key>,
             typename AllocatorType = Allocator<Pair<const Key, Value>>>
    using TreeMultiMap = std::multimap<Key, Value, Compare, AllocatorType>;

    template<typename... Args>
    using Tuple = std::tuple<Args...>;


    template<size_t size>
    using BitSet = std::bitset<size>;

    template<typename Type>
    using SmartPointer = std::shared_ptr<Type>;

    template<typename Signature>
    using Function = std::function<Signature>;

    template<typename Type>
    void fake_delete(Type*)
    {}

    template<typename Type>
    void delete_value(Type* value)
    {
        delete value;
    }

    template<typename Type>
    void delete_array(Type* array)
    {
        delete[] array;
    }

    template<typename Type, typename Dp = std::default_delete<Type>>
    using ScopedPtr = std::unique_ptr<Type, Dp>;

    using String     = std::string;
    using StringView = std::string_view;
}// namespace Engine
