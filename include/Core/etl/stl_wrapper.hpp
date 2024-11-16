#pragma once
#include <Core/etl/allocator.hpp>
#include <Core/etl/any.hpp>
#include <Core/etl/vector.hpp>
#include <array>
#include <atomic>
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


namespace Engine
{
	template<typename Type>
	using Hash = std::hash<Type>;

	template<typename T1, typename T2>
	using Pair = std::pair<T1, T2>;

	template<typename Type, size_t size>
	using Array = std::array<Type, size>;

	template<typename Type>
	using Vector = Containers::Vector<Type, Allocator<Type>>;

	template<typename Type>
	using List = std::list<Type, Allocator<Type>>;

	template<typename Type>
	using ForwardList = std::forward_list<Type, Allocator<Type>>;

	template<typename Type, typename HashType = Hash<Type>, typename Pred = std::equal_to<Type>>
	using Set = std::unordered_set<Type, HashType, Pred, Allocator<Type>>;

	template<typename Type, typename HashType = Hash<Type>, typename Pred = std::equal_to<Type>>
	using MultiSet = std::unordered_multiset<Type, HashType, Pred, Allocator<Type>>;

	template<typename Key, typename Value, typename HashType = Hash<Key>, typename Pred = std::equal_to<Key>>
	using Map = std::unordered_map<Key, Value, HashType, Pred, Allocator<Pair<const Key, Value>>>;

	template<typename Key, typename Value, typename HashType = Hash<Key>, typename Pred = std::equal_to<Key>>
	using MultiMap = std::unordered_multimap<Key, Value, HashType, Pred, Allocator<Pair<const Key, Value>>>;

	template<typename Type, size_t extend = std::dynamic_extent>
	using Span = std::span<Type, extend>;

	template<typename Type, typename Compare = std::less<Type>>
	using TreeSet = std::set<Type, Compare, Allocator<Type>>;

	template<typename Type, typename Compare = std::less<Type>>
	using TreeMultiSet = std::multiset<Type, Compare, Allocator<Type>>;

	template<typename Key, typename Value, typename Compare = std::less<Key>>
	using TreeMap = std::map<Key, Value, Compare, Allocator<Pair<const Key, Value>>>;

	template<typename Key, typename Value, typename Compare = std::less<Key>>
	using TreeMultiMap = std::multimap<Key, Value, Compare, Allocator<Pair<const Key, Value>>>;

	template<typename... Args>
	using Tuple = std::tuple<Args...>;

	template<size_t size>
	using BitSet = std::bitset<size>;

	template<typename Type>
	using SharedPtr = std::shared_ptr<Type>;

	template<typename Signature>
	using Function = std::function<Signature>;

	template<typename Type>
	using Atomic = std::atomic<Type>;

	template<typename Type, typename Dp = std::default_delete<Type>>
	using ScopedPtr = std::unique_ptr<Type, Dp>;

	using String     = std::string;
	using StringView = std::string_view;

	template<typename A, typename B>
	concept is_same_concept = std::is_same_v<A, B>;

	template<typename A, typename B>
	concept is_not_same_concept = !std::is_same_v<A, B>;
}// namespace Engine
