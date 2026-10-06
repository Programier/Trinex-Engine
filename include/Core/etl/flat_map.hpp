#pragma once
#include <Core/etl/flat_set.hpp>
#include <Core/etl/pair.hpp>

namespace Trinex
{
	template<typename Key, typename Value, typename Compare>
	struct FlatMapCompare {
		using is_transparent = void;

		constexpr bool operator()(const Pair<Key, Value>& a, const Pair<Key, Value>& b) const
		{
			return Compare()(a.first, b.first);
		}

		template<typename LookupKey>
		constexpr bool operator()(const Pair<Key, Value>& a, const LookupKey& b) const
		{
			return Compare()(a.first, b);
		}

		template<typename LookupKey>
		constexpr bool operator()(const LookupKey& a, const Pair<Key, Value>& b) const
		{
			return Compare()(a, b.first);
		}
	};

	template<typename Key, typename Value, typename Compare = Less<Key>, typename AllocatorType = Allocator<Pair<Key, Value>>>
	class FlatMap : private FlatSet<Pair<Key, Value>, FlatMapCompare<Key, Value, Compare>, AllocatorType>
	{
		using base_type = FlatSet<Pair<Key, Value>, FlatMapCompare<Key, Value, Compare>, AllocatorType>;

	public:
		using key_type      = Key;
		using mapped_type   = Value;
		using value_type    = Pair<Key, Value>;
		using key_compare   = Compare;
		using value_compare = Compare;

		using container_type         = typename base_type::container_type;
		using reference              = typename base_type::reference;
		using const_reference        = typename base_type::const_reference;
		using pointer                = typename base_type::pointer;
		using const_pointer          = typename base_type::const_pointer;
		using iterator               = typename base_type::iterator;
		using const_iterator         = typename base_type::const_iterator;
		using reverse_iterator       = typename base_type::reverse_iterator;
		using const_reverse_iterator = typename base_type::const_reverse_iterator;
		using size_type              = typename base_type::size_type;
		using difference_type        = typename base_type::difference_type;

		FlatMap() = default;

		constexpr FlatMap(const AllocatorType& allocator) noexcept : base_type(allocator) {}
		constexpr explicit FlatMap(size_type n) : base_type(n) {}
		constexpr explicit FlatMap(size_type n, const AllocatorType& allocator) : base_type(n, allocator) {}

		constexpr FlatMap(size_type n, const value_type& v) : base_type(n, v) {}
		constexpr FlatMap(size_type n, const value_type& v, const AllocatorType& allocator) : base_type(n, v, allocator) {}

		template<class InputIterator, typename = container_type::template RequireInputIter<InputIterator>>
		constexpr FlatMap(InputIterator first, InputIterator last) : base_type(first, last)
		{}

		template<class InputIterator, typename = container_type::template RequireInputIter<InputIterator>>
		constexpr FlatMap(InputIterator first, InputIterator last, const AllocatorType& allocator)
		    : base_type(first, last, allocator)
		{}

		constexpr FlatMap(std::initializer_list<value_type> list) : base_type(list.begin(), list.end()) {}
		constexpr FlatMap(std::initializer_list<value_type> list, const AllocatorType& allocator)
		    : base_type(list.begin(), list.end(), allocator)
		{}

		constexpr FlatMap(const FlatMap& other) : base_type(other.begin(), other.end(), other.allocator()) {}
		constexpr FlatMap(const FlatMap& other, const AllocatorType& allocator) : base_type(other.begin(), other.end(), allocator)
		{}

		constexpr FlatMap(FlatMap&& other) : base_type(std::move(other)) {}
		constexpr FlatMap(FlatMap&& other, const AllocatorType& allocator) : base_type(std::move(other), allocator) {}

		constexpr FlatMap& operator=(const FlatMap& other)
		{
			base_type::operator=(other);
			return *this;
		}

		constexpr FlatMap& operator=(FlatMap&& other)
		{
			base_type::operator=(std::move(other));
			return *this;
		}

		constexpr FlatMap& operator=(std::initializer_list<value_type> list)
		{
			base_type::operator=(list);
			return *this;
		}

		constexpr mapped_type& operator[](const key_type& key)
		{
			auto it = find(key);

			if (it == end())
				it = insert({key, Value()}).first;

			return const_cast<mapped_type&>(it->second);
		}

		using base_type::begin;
		using base_type::capacity;
		using base_type::cbegin;
		using base_type::cend;
		using base_type::clear;
		using base_type::contains;
		using base_type::count;
		using base_type::crbegin;
		using base_type::crend;
		using base_type::empty;
		using base_type::end;
		using base_type::equal_range;
		using base_type::erase;
		using base_type::find;
		using base_type::insert;
		using base_type::lower_bound;
		using base_type::max_size;
		using base_type::rbegin;
		using base_type::rend;
		using base_type::reserve;
		using base_type::size;
		using base_type::upper_bound;

		constexpr void swap(FlatMap& other) { base_type::swap(other); }
		constexpr const container_type& container() const { return base_type::container(); }
	};

}// namespace Trinex
