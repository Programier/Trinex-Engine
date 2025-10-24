#pragma once

#include <Core/etl/functional.hpp>
#include <Core/etl/pair.hpp>
#include <Core/etl/vector.hpp>
#include <algorithm>

namespace Engine
{
	template<typename T, typename Compare = Less<T>, typename AllocatorType = Allocator<T>>
	class FlatSet : protected Vector<T, AllocatorType>
	{
	public:
		using container_type = Vector<T, AllocatorType>;
		using key_type       = T;
		using key_compare    = Compare;
		using value_compare  = Compare;

		using value_type             = typename container_type::value_type;
		using reference              = typename container_type::const_reference;
		using const_reference        = typename container_type::const_reference;
		using pointer                = typename container_type::const_pointer;
		using const_pointer          = typename container_type::const_pointer;
		using iterator               = typename container_type::const_iterator;
		using const_iterator         = typename container_type::const_iterator;
		using reverse_iterator       = typename container_type::const_reverse_iterator;
		using const_reverse_iterator = typename container_type::const_reverse_iterator;

		using size_type       = typename container_type::size_type;
		using difference_type = typename container_type::difference_type;

	private:
		constexpr void sort() { std::sort(container_type::begin(), container_type::end(), Compare()); }

	public:
		FlatSet() = default;

		constexpr FlatSet(const AllocatorType& allocator) noexcept : container_type(allocator) {}
		constexpr explicit FlatSet(size_type n) : container_type(n) { sort(); }
		constexpr explicit FlatSet(size_type n, const AllocatorType& allocator) : container_type(n, allocator) { sort(); }
		constexpr FlatSet(size_type n, const value_type& v) : container_type(n, v) { sort(); }

		constexpr FlatSet(size_type n, const value_type& v, const AllocatorType& allocator) : container_type(n, v, allocator)
		{
			sort();
		}

		template<class InputIterator, typename = container_type::template RequireInputIter<InputIterator>>
		constexpr FlatSet(InputIterator first, InputIterator last) : container_type(first, last)
		{
			sort();
		}

		template<class InputIterator, typename = container_type::template RequireInputIter<InputIterator>>
		constexpr FlatSet(InputIterator first, InputIterator last, const AllocatorType& allocator)
		    : container_type(first, last, allocator)
		{
			sort();
		}

		constexpr FlatSet(std::initializer_list<T> list) : container_type(list.begin(), list.end()) { sort(); }

		constexpr FlatSet(std::initializer_list<T> list, const AllocatorType& allocator)
		    : container_type(list.begin(), list.end(), allocator)
		{}

		constexpr FlatSet(const FlatSet& other) : container_type(other.begin(), other.end(), other.allocator()) {}
		constexpr FlatSet(const FlatSet& other, const AllocatorType& allocator)
		    : container_type(other.begin(), other.end(), allocator)
		{}

		constexpr FlatSet(FlatSet&& other) : container_type(std::move(other)) {}
		constexpr FlatSet(FlatSet&& other, const AllocatorType& allocator) : container_type(std::move(other), allocator) {}

		constexpr FlatSet& operator=(const FlatSet& other)
		{
			container_type::operator=(other);
			return *this;
		}

		constexpr FlatSet& operator=(FlatSet&& other)
		{
			container_type::operator=(std::move(other));
			return *this;
		}

		constexpr FlatSet& operator=(std::initializer_list<T> list)
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
		constexpr void swap(FlatSet& other) { container_type::swap(other); }


		constexpr const_iterator find(const T& value) const
		{
			Compare compare;
			auto it = std::lower_bound(begin(), end(), value, compare);
			return (it != end() && !compare(value, *it)) ? it : end();
		}

		constexpr const_iterator lower_bound(const T& value) const
		{
			Compare compare;
			return std::lower_bound(begin(), end(), value, compare);
		}

		constexpr const_iterator upper_bound(const T& value) const
		{
			Compare compare;
			return std::upper_bound(begin(), end(), value, compare);
		}

		constexpr bool contains(const T& value) const { return find(value) != end(); }

		constexpr Pair<iterator, bool> insert(const T& value)
		{
			Compare compare;
			auto it = std::lower_bound(begin(), end(), value, compare);
			if (it == end() || compare(value, *it))
			{
				return {container_type::insert(it, value), true};
			}
			return {it, false};
		}

		template<class InputIterator, typename = container_type::template RequireInputIter<InputIterator>>
		constexpr void insert(InputIterator first, InputIterator last)
		{
			container_type::insert(container_type::end(), first, last);
			sort();
			container_type::erase(std::unique(container_type::begin(), container_type::end()), container_type::end());
		}

		constexpr const_iterator erase(const T& value)
		{
			auto it = find(value);
			if (it != end())
				return container_type::erase(it);
			return end();
		}

		constexpr const_iterator erase(const_iterator it) { return container_type::erase(it); }
		constexpr const_iterator erase(const_iterator from, const_iterator to) { return container_type::erase(from, to); }
		constexpr const Vector<T, AllocatorType>& as_vector() const { return *this; }
	};


	template<typename T, typename Compare = Less<T>, typename AllocatorType = Allocator<T>, typename ArchiveType>
	inline bool trinex_serialize_flat_set(ArchiveType& ar, FlatSet<T, Compare, AllocatorType>& set)
	    requires(is_complete_archive_type<ArchiveType>)
	{
		const Vector<T, AllocatorType>& vector = set.as_vector();
		return ar.serialize_vector(const_cast<Vector<T, AllocatorType>&>(vector));
	}

	template<typename T, typename C, typename A>
	struct Serializer<FlatSet<T, C, A>> {
		bool serialize(Archive& ar, FlatSet<T, C, A>& set) { return trinex_serialize_flat_set(ar, set); }
	};
}// namespace Engine
