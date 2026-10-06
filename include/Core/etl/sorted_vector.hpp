#pragma once

#include <Core/etl/functional.hpp>
#include <Core/etl/pair.hpp>
#include <Core/etl/vector.hpp>
#include <type_traits>

namespace Trinex
{
	template<typename T, typename Compare = Less<T>, typename AllocatorType = Allocator<T>>
	class SortedVector : protected Vector<T, AllocatorType>
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

	protected:
		template<typename LookupKey>
		static constexpr bool is_lookup_key =
		        std::is_convertible_v<const LookupKey&, key_type> ||
		        (requires { typename Compare::is_transparent; } &&
		         requires(Compare compare, const key_type& key, const LookupKey& lookup_key) {
			         compare(key, lookup_key);
			         compare(lookup_key, key);
		         });

		constexpr void sort() { etl::sort(container_type::begin(), container_type::end(), Compare()); }

	public:
		SortedVector() = default;

		constexpr SortedVector(const AllocatorType& allocator) noexcept : container_type(allocator) {}

		constexpr explicit SortedVector(size_type n) : container_type(n) { sort(); }

		constexpr explicit SortedVector(size_type n, const AllocatorType& allocator) : container_type(n, allocator) { sort(); }

		constexpr SortedVector(size_type n, const value_type& value) : container_type(n, value) {}

		constexpr SortedVector(size_type n, const value_type& value, const AllocatorType& allocator)
		    : container_type(n, value, allocator)
		{}

		template<class InputIterator, typename = container_type::template RequireInputIter<InputIterator>>
		constexpr SortedVector(InputIterator first, InputIterator last) : container_type(first, last)
		{
			sort();
		}

		template<class InputIterator, typename = container_type::template RequireInputIter<InputIterator>>
		constexpr SortedVector(InputIterator first, InputIterator last, const AllocatorType& allocator)
		    : container_type(first, last, allocator)
		{
			sort();
		}

		constexpr SortedVector(std::initializer_list<T> list) : container_type(list.begin(), list.end()) { sort(); }

		constexpr SortedVector(std::initializer_list<T> list, const AllocatorType& allocator)
		    : container_type(list.begin(), list.end(), allocator)
		{
			sort();
		}

		constexpr SortedVector(const SortedVector& other) : container_type(other.begin(), other.end(), other.allocator()) {}

		constexpr SortedVector(const SortedVector& other, const AllocatorType& allocator)
		    : container_type(other.begin(), other.end(), allocator)
		{}

		constexpr SortedVector(SortedVector&& other) : container_type(etl::move(other)) {}

		constexpr SortedVector(SortedVector&& other, const AllocatorType& allocator) : container_type(etl::move(other), allocator)
		{}

		constexpr SortedVector& operator=(const SortedVector& other)
		{
			container_type::operator=(other);
			return *this;
		}

		constexpr SortedVector& operator=(SortedVector&& other)
		{
			container_type::operator=(etl::move(other));
			return *this;
		}

		constexpr SortedVector& operator=(std::initializer_list<T> list)
		{
			container_type::assign(list.begin(), list.end());
			sort();
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

		constexpr void swap(SortedVector& other) { container_type::swap(other); }

		template<typename LookupKey>
		    requires(is_lookup_key<LookupKey>)
		constexpr const_iterator find(const LookupKey& value) const
		{
			Compare compare;

			auto it = etl::lower_bound(begin(), end(), value, compare);

			if (it != end() && !compare(value, *it) && !compare(*it, value))
				return it;

			return end();
		}

		template<typename LookupKey>
		    requires(is_lookup_key<LookupKey>)
		constexpr bool contains(const LookupKey& value) const
		{
			return find(value) != end();
		}

		template<typename LookupKey>
		    requires(is_lookup_key<LookupKey>)
		constexpr size_type count(const LookupKey& value) const
		{
			auto range = equal_range(value);
			return static_cast<size_type>(range.second - range.first);
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

		constexpr iterator insert(const value_type& value)
		{
			auto it = etl::upper_bound(begin(), end(), value, Compare());
			return container_type::insert(it, value);
		}

		constexpr iterator insert(value_type&& value)
		{
			auto it = etl::upper_bound(begin(), end(), value, Compare());
			return container_type::insert(it, etl::move(value));
		}

		template<class InputIterator, typename = container_type::template RequireInputIter<InputIterator>>
		constexpr void insert(InputIterator first, InputIterator last)
		{
			container_type::insert(container_type::end(), first, last);
			sort();
		}

		template<typename... Args>
		constexpr iterator emplace(Args&&... args)
		{
			value_type value(std::forward<Args>(args)...);
			auto it = etl::upper_bound(begin(), end(), value, Compare());
			return container_type::insert(it, etl::move(value));
		}

		template<typename LookupKey>
		    requires(is_lookup_key<LookupKey>)
		constexpr size_type erase(const LookupKey& value)
		{
			auto range = equal_range(value);

			if (range.first == range.second)
				return 0;

			const size_type count = static_cast<size_type>(range.second - range.first);

			container_type::erase(range.first, range.second);

			return count;
		}

		constexpr const_iterator erase(const_iterator it) { return container_type::erase(it); }

		constexpr const_iterator erase(const_iterator first, const_iterator last) { return container_type::erase(first, last); }

		constexpr const container_type& as_vector() const { return *this; }

		constexpr const container_type& container() const { return *this; }
	};


	template<typename T, typename Compare = Less<T>, typename AllocatorType = Allocator<T>, typename ArchiveType>
	inline bool trinex_serialize_sorted_vector(ArchiveType& ar, SortedVector<T, Compare, AllocatorType>& vector)
	    requires(is_complete_archive_type<ArchiveType>)
	{
		const Vector<T, AllocatorType>& container = vector.as_vector();
		const bool result                         = ar.serialize_vector(const_cast<Vector<T, AllocatorType>&>(container));
		return result;
	}

	template<typename T, typename C, typename A>
	struct Serializer<SortedVector<T, C, A>> {
		bool serialize(Archive& ar, SortedVector<T, C, A>& vector) { return trinex_serialize_sorted_vector(ar, vector); }
	};
}// namespace Trinex
