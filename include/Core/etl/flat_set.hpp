#pragma once

#include <Core/etl/functional.hpp>
#include <Core/etl/pair.hpp>
#include <Core/etl/array.hpp>
#include <type_traits>

namespace Trinex
{
	template<typename T, typename Compare = Less<T>, typename AllocatorType = Allocator<T>>
	class FlatSet : protected Array<T, AllocatorType>
	{
	public:
		using container_type = Array<T, AllocatorType>;
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

	protected:
		template<typename LookupKey>
		static constexpr bool is_lookup_key =
		        std::is_convertible_v<const LookupKey&, key_type> ||
		        (requires { typename Compare::is_transparent; } &&
		         requires(Compare compare, const key_type& key, const LookupKey& lookup_key) {
			         compare(key, lookup_key);
			         compare(lookup_key, key);
		         });

		static constexpr bool equivalent(const value_type& a, const value_type& b, Compare compare = Compare())
		{
			return !compare(a, b) && !compare(b, a);
		}

		constexpr void sort_and_unique()
		{
			Compare compare;

			etl::sort(container_type::begin(), container_type::end(), compare);

			auto last_unique =
			        etl::unique(container_type::begin(), container_type::end(),
			                    [&](const value_type& a, const value_type& b) { return !compare(a, b) && !compare(b, a); });

			container_type::erase(last_unique, container_type::end());
		}

	public:
		FlatSet() = default;

		constexpr FlatSet(const AllocatorType& allocator) noexcept : container_type(allocator) {}

		constexpr explicit FlatSet(size_type n) : container_type(n) { sort_and_unique(); }

		constexpr explicit FlatSet(size_type n, const AllocatorType& allocator) : container_type(n, allocator)
		{
			sort_and_unique();
		}

		constexpr FlatSet(size_type n, const value_type& v) : container_type(n, v) { sort_and_unique(); }

		constexpr FlatSet(size_type n, const value_type& v, const AllocatorType& allocator) : container_type(n, v, allocator)
		{
			sort_and_unique();
		}

		template<class InputIterator, typename = container_type::template RequireInputIter<InputIterator>>
		constexpr FlatSet(InputIterator first, InputIterator last) : container_type(first, last)
		{
			sort_and_unique();
		}

		template<class InputIterator, typename = container_type::template RequireInputIter<InputIterator>>
		constexpr FlatSet(InputIterator first, InputIterator last, const AllocatorType& allocator)
		    : container_type(first, last, allocator)
		{
			sort_and_unique();
		}

		constexpr FlatSet(std::initializer_list<T> list) : container_type(list.begin(), list.end()) { sort_and_unique(); }

		constexpr FlatSet(std::initializer_list<T> list, const AllocatorType& allocator)
		    : container_type(list.begin(), list.end(), allocator)
		{
			sort_and_unique();
		}

		constexpr FlatSet(const FlatSet& other) : container_type(other.begin(), other.end(), other.allocator()) {}

		constexpr FlatSet(const FlatSet& other, const AllocatorType& allocator)
		    : container_type(other.begin(), other.end(), allocator)
		{}

		constexpr FlatSet(FlatSet&& other) : container_type(etl::move(other)) {}

		constexpr FlatSet(FlatSet&& other, const AllocatorType& allocator) : container_type(etl::move(other), allocator) {}

		constexpr FlatSet& operator=(const FlatSet& other)
		{
			container_type::operator=(other);
			return *this;
		}

		constexpr FlatSet& operator=(FlatSet&& other)
		{
			container_type::operator=(etl::move(other));
			return *this;
		}

		constexpr FlatSet& operator=(std::initializer_list<T> list)
		{
			container_type::assign(list.begin(), list.end());
			sort_and_unique();
			return *this;
		}

		constexpr const_iterator begin() const { return container_type::cbegin(); }
		constexpr const_iterator end() const { return container_type::cend(); }
		constexpr const_iterator cbegin() const { return container_type::cbegin(); }
		constexpr const_iterator cend() const { return container_type::cend(); }
		constexpr const_reverse_iterator rbegin() const { return container_type::crbegin(); }
		constexpr const_reverse_iterator rend() const { return container_type::crend(); }
		constexpr const_reverse_iterator crbegin() const { return container_type::crbegin(); }
		constexpr const_reverse_iterator crend() const { return container_type::crend(); }

		constexpr bool empty() const { return container_type::empty(); }
		constexpr size_type size() const { return container_type::size(); }
		constexpr size_type capacity() const { return container_type::capacity(); }
		constexpr size_type max_size() const { return container_type::max_size(); }
		constexpr void reserve(size_type size) { container_type::reserve(size); }
		constexpr void clear() { container_type::clear(); }
		constexpr void swap(FlatSet& other) { container_type::swap(other); }


		template<typename LookupKey>
		    requires(is_lookup_key<LookupKey>)
		constexpr const_iterator find(const LookupKey& value) const
		{
			Compare compare;

			auto it = etl::lower_bound(begin(), end(), value, compare);

			return (it != end() && !compare(value, *it)) ? it : end();
		}

		template<typename LookupKey>
		    requires(is_lookup_key<LookupKey>)
		constexpr size_type count(const LookupKey& value) const
		{
			return contains(value) ? 1 : 0;
		}

		template<typename LookupKey>
		    requires(is_lookup_key<LookupKey>)
		constexpr bool contains(const LookupKey& value) const
		{
			return find(value) != end();
		}

		template<typename LookupKey>
		    requires(is_lookup_key<LookupKey>)
		constexpr const_iterator lower_bound(const LookupKey& value) const
		{
			return etl::lower_bound(begin(), end(), value, Compare());
		}

		template<typename LookupKey>
		    requires(is_lookup_key<LookupKey>)
		constexpr const_iterator upper_bound(const LookupKey& value) const
		{
			return etl::upper_bound(begin(), end(), value, Compare());
		}

		template<typename LookupKey>
		    requires(is_lookup_key<LookupKey>)
		constexpr Pair<const_iterator, const_iterator> equal_range(const LookupKey& value) const
		{
			auto range = etl::equal_range(begin(), end(), value, Compare());
			return {range.first, range.second};
		}

		constexpr Pair<iterator, bool> insert(const T& value)
		{
			Compare compare;

			auto it = etl::lower_bound(begin(), end(), value, compare);

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
			sort_and_unique();
		}

		template<typename LookupKey>
		    requires(is_lookup_key<LookupKey>)
		constexpr const_iterator erase(const LookupKey& value)
		{
			auto it = find(value);
			if (it != end())
				return container_type::erase(it);
			return end();
		}

		constexpr const_iterator erase(const_iterator it) { return container_type::erase(it); }
		constexpr const_iterator erase(const_iterator from, const_iterator to) { return container_type::erase(from, to); }
		constexpr const container_type& container() const { return *this; }
	};


	template<typename T, typename Compare = Less<T>, typename AllocatorType = Allocator<T>, typename ArchiveType>
	inline bool trinex_serialize_flat_set(ArchiveType& ar, FlatSet<T, Compare, AllocatorType>& set)
	    requires(is_complete_archive_type<ArchiveType>)
	{
		const Array<T, AllocatorType>& vector = set.container();
		return ar.serialize_vector(const_cast<Array<T, AllocatorType>&>(vector));
	}

	template<typename T, typename C, typename A>
	struct Serializer<FlatSet<T, C, A>> {
		bool serialize(Archive& ar, FlatSet<T, C, A>& set) { return trinex_serialize_flat_set(ar, set); }
	};
}// namespace Trinex
