#pragma once
#include <Core/etl/pair.hpp>
#include <Core/etl/vector.hpp>
#include <algorithm>
#include <functional>

namespace Engine
{
	template<typename Key, typename Value, typename Compare = std::less<Key>,
	         typename AllocatorType = Allocator<Pair<Key, Value>>>
	class FlatMap : private Vector<Pair<Key, Value>, AllocatorType>
	{
	public:
		using key_type      = Key;
		using mapped_type   = Value;
		using value_type    = Pair<Key, Value>;
		using key_compare   = Compare;
		using value_compare = Compare;

		using container_type         = Vector<value_type, AllocatorType>;
		using reference              = typename container_type::const_reference;
		using const_reference        = typename container_type::const_reference;
		using pointer                = typename container_type::const_pointer;
		using const_pointer          = typename container_type::const_pointer;
		using iterator               = typename container_type::const_iterator;
		using const_iterator         = typename container_type::const_iterator;
		using reverse_iterator       = typename container_type::const_reverse_iterator;
		using const_reverse_iterator = typename container_type::const_reverse_iterator;
		using size_type              = typename container_type::size_type;
		using difference_type        = typename container_type::difference_type;

	private:
		constexpr void sort()
		{
			std::sort(container_type::begin(), container_type::end(),
			          [](const value_type& a, const value_type& b) { return Compare()(a.first, b.first); });
		}

	public:
		FlatMap() = default;

		constexpr FlatMap(const AllocatorType& allocator) noexcept : container_type(allocator) {}
		constexpr explicit FlatMap(size_type n) : container_type(n) { sort(); }
		constexpr explicit FlatMap(size_type n, const AllocatorType& allocator) : container_type(n, allocator) { sort(); }

		constexpr FlatMap(size_type n, const value_type& v) : container_type(n, v) { sort(); }
		constexpr FlatMap(size_type n, const value_type& v, const AllocatorType& allocator) : container_type(n, v, allocator)
		{
			sort();
		}

		template<class InputIterator, typename = container_type::template RequireInputIter<InputIterator>>
		constexpr FlatMap(InputIterator first, InputIterator last) : container_type(first, last)
		{
			sort();
		}

		template<class InputIterator, typename = container_type::template RequireInputIter<InputIterator>>
		constexpr FlatMap(InputIterator first, InputIterator last, const AllocatorType& allocator)
		    : container_type(first, last, allocator)
		{
			sort();
		}

		constexpr FlatMap(std::initializer_list<value_type> list) : container_type(list.begin(), list.end()) { sort(); }

		constexpr FlatMap(std::initializer_list<value_type> list, const AllocatorType& allocator)
		    : container_type(list.begin(), list.end(), allocator)
		{
			sort();
		}

		constexpr FlatMap(const FlatMap& other) : container_type(other.begin(), other.end(), other.allocator()) {}
		constexpr FlatMap(const FlatMap& other, const AllocatorType& allocator)
		    : container_type(other.begin(), other.end(), allocator)
		{}

		constexpr FlatMap(FlatMap&& other) : container_type(std::move(other)) {}
		constexpr FlatMap(FlatMap&& other, const AllocatorType& allocator) : container_type(std::move(other), allocator) {}

		constexpr FlatMap& operator=(const FlatMap& other)
		{
			container_type::operator=(other);
			return *this;
		}

		constexpr FlatMap& operator=(FlatMap&& other)
		{
			container_type::operator=(std::move(other));
			return *this;
		}

		constexpr FlatMap& operator=(std::initializer_list<value_type> list)
		{
			container_type::assign(list.begin(), list.end());
			sort();
			return *this;
		}

		constexpr inline const_iterator begin() const { return container_type::cbegin(); }
		constexpr inline const_iterator end() const { return container_type::cend(); }
		constexpr inline const_iterator cbegin() const { return container_type::cbegin(); }
		constexpr inline const_iterator cend() const { return container_type::cend(); }
		constexpr inline const_reverse_iterator rbegin() const { return container_type::crbegin(); }
		constexpr inline const_reverse_iterator rend() const { return container_type::crend(); }
		constexpr inline const_reverse_iterator crbegin() const { return container_type::crbegin(); }
		constexpr inline const_reverse_iterator crend() const { return container_type::crend(); }

		constexpr bool empty() const { return container_type::empty(); }
		constexpr size_type size() const { return container_type::size(); }
		constexpr size_type capacity() const { return container_type::capacity(); }
		constexpr size_type max_size() const { return container_type::max_size(); }
		constexpr void reserve(size_type size) { container_type::reserve(size); }
		constexpr void clear() { container_type::clear(); }
		constexpr void swap(FlatMap& other) { container_type::swap(other); }

		constexpr const_iterator find(const key_type& key) const
		{
			auto it = std::lower_bound(begin(), end(), key,
			                           [&](const value_type& elem, const key_type& val) { return Compare()(elem.first, val); });
			if (it != end() && !Compare()(key, it->first))
				return it;
			return end();
		}

		constexpr const_iterator lower_bound(const key_type& key) const
		{
			return std::lower_bound(begin(), end(), key,
			                        [&](const value_type& elem, const key_type& val) { return Compare()(elem.first, val); });
		}

		constexpr const_iterator upper_bound(const key_type& key) const
		{
			return std::upper_bound(begin(), end(), key,
			                        [&](const value_type& elem, const key_type& val) { return Compare()(elem.first, val); });
		}

		constexpr bool contains(const key_type& key) const { return find(key) != end(); }

		constexpr Pair<iterator, bool> insert(const value_type& value)
		{
			auto it = std::lower_bound(begin(), end(), value.first,
			                           [&](const value_type& elem, const key_type& val) { return Compare()(elem.first, val); });
			if (it == end() || Compare()(value.first, it->first))
				return {container_type::insert(it, value), true};
			return {it, false};
		}

		template<class InputIterator, typename = container_type::template RequireInputIter<InputIterator>>
		constexpr void insert(InputIterator first, InputIterator last)
		{
			container_type::insert(container_type::end(), first, last);
			sort();
			auto last_unique = std::unique(container_type::begin(), container_type::end(),
			                               [](const value_type& a, const value_type& b) { return a.first == b.first; });
			container_type::erase(last_unique, container_type::end());
		}

		constexpr const_iterator erase(const key_type& key)
		{
			auto it = find(key);
			if (it != end())
				return container_type::erase(it);
			return end();
		}

		constexpr const_iterator erase(const_iterator it) { return container_type::erase(it); }
		constexpr const_iterator erase(const_iterator first, const_iterator last) { return container_type::erase(first, last); }
	};

}// namespace Engine
