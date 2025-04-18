#pragma once
#include <Core/etl/allocator.hpp>
#include <Core/etl/archive_predef.hpp>
#include <algorithm>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>

namespace Engine::Containers
{
	template<typename T, typename AllocatorType = std::allocator<T>>
	class Vector : private AllocatorType
	{
	public:
		using value_type             = T;
		using reference              = T&;
		using const_reference        = const T&;
		using pointer                = T*;
		using const_pointer          = const T*;
		using iterator               = T*;
		using const_iterator         = const T*;
		using reverse_iterator       = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;

	protected:
		pointer m_start;
		pointer m_finish;
		pointer m_end;

	private:
		template<typename IteratorType>
		using IteratorCategory = typename std::iterator_traits<IteratorType>::iterator_category;

		template<typename IteratorType>
		using RequireInputIter =
		        std::enable_if_t<std::is_convertible<IteratorCategory<IteratorType>, std::input_iterator_tag>::value>;

		template<typename IteratorType>
		using RequireForwardIter =
		        std::enable_if_t<std::is_convertible<IteratorCategory<IteratorType>, std::forward_iterator_tag>::value>;

		template<typename IteratorType>
		static constexpr inline bool is_forward_iterator =
		        std::is_convertible_v<typename std::iterator_traits<IteratorType>::iterator_category, std::forward_iterator_tag>;

		constexpr AllocatorType& allocator() { return *this; }

		constexpr const AllocatorType& allocator() const { return *this; }

		constexpr inline void range_check(size_type n) const
		{
			if (n >= size())
				throw std::out_of_range("Index out of range");
		}

		template<typename Ptr>
		constexpr inline typename std::pointer_traits<Ptr>::element_type* data_ptr(Ptr ptr) const
		{
			return empty() ? nullptr : std::to_address(ptr);
		}

		constexpr size_type next_capacity(size_type n = 1)
		{
			size_type c = capacity();
			if (c == 0)
				c = 1;
			while (c < size() + n) c *= 2;
			return c;
		}

		pointer allocate_and_copy(size_type count, const_pointer first, const_pointer last)
		{
			pointer result = allocator().allocate(count);
			try
			{
				std::uninitialized_copy(first, last, result);
				return result;
			}
			catch (...)
			{
				allocator().deallocate(result, count);
				throw;
			}
		}

		constexpr void default_append(size_type n)
		{
			if (n != 0)
			{
				const size_type s = size();
				size_type avail   = size_type(m_end - m_finish);

				if (avail >= n)
				{
					m_finish = std::uninitialized_default_construct_n(m_finish, n);
				}
				else
				{
					const size_type len = next_capacity(n);
					pointer new_start(allocator().allocate(len));
					pointer destroy_from = pointer();

					try
					{
						std::uninitialized_default_construct_n(new_start + s, n);
						destroy_from = new_start + s;
						std::uninitialized_move(m_start, m_finish, new_start);
					}
					catch (...)
					{
						if (destroy_from)
							std::destroy(destroy_from, destroy_from + n);

						allocator().deallocate(new_start, len);
						throw;
					}

					allocator().deallocate(m_start, capacity());
					m_start  = new_start;
					m_finish = new_start + s + n;
					m_end    = new_start + len;
				}
			}
		}

		constexpr void fill_insert(iterator pos, size_type n, const value_type& x)
		{
			if (n != 0)
			{
				if (size_type(m_end - m_finish) >= n)
				{
					value_type x_copy = value_type(x);

					const size_type elems_after = end() - pos;
					pointer old_finish(m_finish);

					if (elems_after > n)
					{
						std::uninitialized_move(old_finish - n, old_finish, old_finish);
						m_finish += n;
						std::move_backward(pos, old_finish - n, old_finish);
						std::fill(pos, pos + n, x_copy);
					}
					else
					{
						m_finish = std::uninitialized_fill_n(old_finish, n - elems_after, x_copy);
						std::uninitialized_move(pos, old_finish, m_finish);
						m_finish += elems_after;
						std::fill(pos, old_finish, x_copy);
					}
				}
				else
				{
					pointer old_start  = m_start;
					pointer old_finish = m_finish;

					const size_type len          = next_capacity(n);
					const size_type elems_before = pos - old_start;
					pointer new_start(allocator().allocate(len));
					pointer new_finish(new_start);

					try
					{
						std::uninitialized_fill_n(new_start + elems_before, n, x);
						new_finish = nullptr;
						new_finish = std::uninitialized_move(old_start, pos, new_start);
						new_finish += n;
						new_finish = std::uninitialized_move(pos, old_finish, new_finish);
					}
					catch (...)
					{
						if (!new_finish)
							std::destroy(new_start + elems_before, new_start + elems_before + n);
						else
							std::destroy(new_start, new_finish);
						allocator().deallocate(new_start, len);
						throw;
					}
					std::destroy(old_start, old_finish);
					allocator().deallocate(old_start, capacity());
					m_start  = new_start;
					m_finish = new_finish;
					m_end    = new_start + len;
				}
			}
		}

		template<typename Type, typename... Args>
		constexpr auto construct_at(Type* loc, Args&&... args) noexcept(noexcept(::new((void*) 0) Type(std::declval<Args>()...)))
		        -> decltype(::new((void*) 0) Type(std::declval<Args>()...))
		{
			return ::new ((void*) loc) Type(std::forward<Args>(args)...);
		}

		template<typename... Args>
		constexpr inline void realloc_insert(iterator pos, Args&&... args)
		{
			const size_type len          = next_capacity(size_type(1));
			pointer old_start            = m_start;
			pointer old_finish           = m_finish;
			const size_type elems_before = pos - begin();
			pointer new_start(allocator().allocate(len));
			pointer new_finish(new_start);

			try
			{
				construct_at(new_start + elems_before, std::forward<Args>(args)...);
				new_finish = nullptr;
				new_finish = std::uninitialized_move(old_start, pos, new_start) + 1;
				new_finish = std::uninitialized_move(pos, old_finish, new_finish);
			}
			catch (...)
			{
				if (!new_finish)
					std::destroy_at(new_start + elems_before);
				else
					std::destroy(new_start, new_finish);
				allocator().deallocate(new_start, len);
				throw;
			}

			std::destroy(old_start, old_finish);
			allocator().deallocate(old_start, capacity());
			m_start  = new_start;
			m_finish = new_finish;
			m_end    = new_start + len;
		}


		template<typename IteratorType>
		constexpr void range_insert(iterator pos, IteratorType first, IteratorType last, std::input_iterator_tag)
		{
			if (pos == end())
			{
				for (; first != last; ++first) insert(end(), *first);
			}
			else if (first != last)
			{
				Vector tmp(first, last);
				insert(pos, std::make_move_iterator(tmp.begin()), std::make_move_iterator(tmp.end()));
			}
		}

		template<typename IteratorType>
		constexpr void range_insert(iterator pos, IteratorType first, IteratorType last, std::forward_iterator_tag)
		{
			if (first != last)
			{
				const size_type n = std::distance(first, last);
				if (size_type(m_end - m_finish) >= n)
				{
					const size_type elems_after = end() - pos;
					pointer old_finish(m_finish);
					if (elems_after > n)
					{
						std::uninitialized_move(m_finish - n, m_finish, m_finish);
						m_finish += n;
						std::move_backward(pos, old_finish - n, old_finish);
						std::copy(first, last, pos);
					}
					else
					{
						IteratorType mid = first;
						std::advance(mid, elems_after);
						std::uninitialized_copy(mid, last, m_finish);
						m_finish += n - elems_after;
						std::uninitialized_move(pos, old_finish, m_finish);
						m_finish += elems_after;
						std::copy(first, mid, pos);
					}
				}
				else
				{
					pointer old_start  = m_start;
					pointer old_finish = m_finish;

					const size_type len = next_capacity(n);
					pointer new_start(allocator().allocate(len));
					pointer new_finish(new_start);

					try
					{
						new_finish = std::uninitialized_move(old_start, pos, new_start);
						new_finish = std::uninitialized_copy(first, last, new_finish);
						new_finish = std::uninitialized_move(pos, old_finish, new_finish);
					}
					catch (...)
					{
						std::destroy(new_start, new_finish);
						allocator().deallocate(m_start, len);
						throw;
					}

					std::destroy(m_start, m_finish);
					allocator().deallocate(m_start, capacity());
					m_start  = new_start;
					m_finish = new_finish;
					m_end    = new_start + len;
				}
			}
		}

		constexpr auto insert_rval(const_iterator pos, value_type&& v) -> iterator
		{
			const auto n = pos - cbegin();
			if (m_finish != m_end)
				if (pos == cend())
				{
					construct_at(m_finish, std::move(v));
					++m_finish;
				}
				else
					insert_aux(begin() + n, std::move(v));
			else
				realloc_insert(begin() + n, std::move(v));

			return iterator(m_start + n);
		}

		template<typename InputIterator>
		constexpr void assign_aux(InputIterator first, InputIterator last, std::input_iterator_tag)
		{
			std::destroy(m_start, m_end);

			pointer cur(m_start);
			for (; first != last && cur != m_finish; ++cur, ++first) construct_at(cur, *first);

			if (first == last)
				erase_at_end(cur);
			else
				range_insert(end(), first, last);
		}

		template<typename ForwardIterator>
		constexpr void assign_aux(ForwardIterator first, ForwardIterator last, std::forward_iterator_tag)
		{
			const size_type len = std::distance(first, last);

			if (len > capacity())
			{
				pointer tmp(allocate_and_copy(len, first, last));
				std::destroy(m_start, m_finish);
				allocator().deallocate(m_start, capacity());
				m_start  = tmp;
				m_finish = m_start + len;
				m_end    = m_finish;
			}
			else if (size() >= len)
				erase_at_end(std::copy(first, last, m_start));
			else
			{
				ForwardIterator mid = first;
				std::advance(mid, size());
				std::copy(first, mid, m_start);
				m_finish = std::uninitialized_copy(mid, last, m_finish);
			}
		}

		template<typename Arg>
		constexpr void insert_aux(iterator pos, Arg&& arg)
		{
			construct_at(m_finish, std::move(*(m_finish - 1)));
			++m_finish;
			std::move_backward(pos, m_finish - 2, m_finish - 1);
			*pos = std::forward<Arg>(arg);
		}

		template<typename... Args>
		constexpr auto emplace_aux(const_iterator pos, Args&&... args) -> iterator
		{
			const auto n = pos - cbegin();
			if (m_finish != m_end)
				if (pos == cend())
				{
					construct_at(m_finish, std::forward<Args>(args)...);
					++m_finish;
				}
				else
				{
					value_type tmp(std::forward<Args>(args)...);
					insert_aux(begin() + n, std::move(tmp));
				}
			else
				realloc_insert(begin() + n, std::forward<Args>(args)...);

			return iterator(m_start + n);
		}

		constexpr iterator emplace_aux(const_iterator pos, value_type&& v) { return insert_rval(pos, std::move(v)); }

		constexpr inline void append_to_end(size_type count)
		{
			pointer new_finish = m_finish + count;

			while (m_finish != new_finish)
			{
				construct_at(m_finish);
				++m_finish;
			}
		}

		constexpr inline void append_to_end(size_type count, const value_type& v)
		{
			pointer new_finish = m_finish + count;

			while (m_finish != new_finish)
			{
				construct_at(m_finish, v);
				++m_finish;
			}
		}

		constexpr inline void erase_at_end(iterator pos)
		{
			if (size_type n = m_finish - pos)
			{
				std::destroy(pos, m_finish);
				m_finish = pos;
			}
		}

		template<typename InputIterator>
		constexpr inline void range_initialize(InputIterator first, InputIterator last)
		    requires(!is_forward_iterator<InputIterator>)
		{
			try
			{
				for (; first != last; ++first) emplace_back(*first);
			}
			catch (...)
			{
				clear();
				throw;
			}
		}

		template<typename InputIterator>
		constexpr inline void range_initialize(InputIterator first, InputIterator last)
		    requires(is_forward_iterator<InputIterator>)
		{
			const size_type n = std::distance(first, last);
			reserve(n);
			std::uninitialized_copy(first, last, m_start);
			m_finish = m_start + n;
		}

		constexpr void destroy()
		{
			if (m_start)
			{
				clear();
				allocator().deallocate(m_start, capacity());
			}
		}

	public:
		constexpr inline iterator begin() { return m_start; }

		constexpr inline const_iterator begin() const { return m_start; }

		constexpr inline iterator end() { return m_finish; }

		constexpr inline const_iterator end() const { return m_finish; }

		constexpr inline const_iterator cbegin() const { return m_start; }

		constexpr inline const_iterator cend() const { return m_finish; }

		constexpr inline reverse_iterator rbegin() { return reverse_iterator(end()); }

		constexpr inline const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

		constexpr inline reverse_iterator rend() { return reverse_iterator(begin()); }

		constexpr inline const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

		constexpr inline const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }

		constexpr inline const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

		constexpr Vector() noexcept : m_start(nullptr), m_finish(nullptr), m_end(nullptr) {}

		constexpr Vector(const AllocatorType& allocator) noexcept
		    : AllocatorType(allocator), m_start(nullptr), m_finish(nullptr), m_end(nullptr)
		{}

		constexpr explicit Vector(size_type n) : Vector()
		{
			reserve(n);
			append_to_end(n);
		}

		constexpr explicit Vector(size_type n, const AllocatorType& allocator) : Vector(allocator)
		{
			reserve(n);
			append_to_end(n);
		}

		constexpr Vector(size_type n, const value_type& v) : Vector()
		{
			reserve(n);
			append_to_end(n, v);
		}

		constexpr Vector(size_type n, const value_type& v, const AllocatorType& allocator) : Vector(allocator)
		{
			reserve(n);
			append_to_end(n, v);
		}

		template<class InputIterator, typename = RequireInputIter<InputIterator>>
		constexpr Vector(InputIterator first, InputIterator last) : Vector()
		{
			range_initialize(first, last);
		}

		template<class InputIterator, typename = RequireInputIter<InputIterator>>
		constexpr Vector(InputIterator first, InputIterator last, const AllocatorType& allocator) : Vector(allocator)
		{
			range_initialize(first, last);
		}

		constexpr Vector(std::initializer_list<T> list) : Vector(list.begin(), list.end()) {}

		constexpr Vector(std::initializer_list<T> list, const AllocatorType& allocator)
		    : Vector(list.begin(), list.end(), allocator)
		{}

		constexpr Vector(const Vector& other) : Vector(other.begin(), other.end(), other.allocator()) {}

		constexpr Vector(const Vector& other, const AllocatorType& allocator) : Vector(other.begin(), other.end(), allocator) {}

		constexpr Vector(Vector&& other)
		    : AllocatorType(std::move(other)), m_start(other.m_start), m_finish(other.m_finish), m_end(other.m_end)
		{
			other.m_start = other.m_finish = other.m_end = nullptr;
		}

		constexpr Vector(Vector&& other, const AllocatorType& allocator)
		    : AllocatorType(allocator), m_start(other.m_start), m_finish(other.m_finish), m_end(other.m_end)
		{
			other.m_start = other.m_finish = other.m_end = nullptr;
		}

		constexpr ~Vector() { destroy(); }

		constexpr Vector& operator=(const Vector& other)
		{
			if (&other != this)
			{
				assign(other.begin(), other.end());
			}
			return *this;
		}

		constexpr Vector& operator=(Vector&& other)
		{
			if (&other != this)
			{
				destroy();
				m_start  = other.m_start;
				m_end    = other.m_end;
				m_finish = other.m_finish;

				other.m_start = other.m_end = other.m_finish = nullptr;
			}
			return *this;
		}

		constexpr Vector& operator=(std::initializer_list<T> list)
		{
			assign(list.begin(), list.end());
			return *this;
		}

		constexpr reference operator[](size_type n) { return *(m_start + n); }

		constexpr const_reference operator[](size_type n) const { return *(m_start + n); }

		constexpr reference at(size_type n)
		{
			range_check(n);
			return (*this)[n];
		}

		constexpr const_reference at(size_type n) const
		{
			range_check(n);
			return (*this)[n];
		}

		constexpr reference front() { return *begin(); }

		constexpr const_reference front() const { return *begin(); }

		constexpr reference back() { return *(end() - 1); }

		constexpr const_reference back() const { return *(end() - 1); }

		constexpr pointer data() noexcept { return data_ptr(m_start); }

		constexpr const_pointer data() const noexcept { return data_ptr(m_start); }

		constexpr inline bool empty() const { return m_start == m_finish; }

		constexpr inline size_type size() const { return m_finish - m_start; }

		constexpr inline size_type max_size() const
		{
			const size_type diffmax  = std::numeric_limits<difference_type>::max() / sizeof(value_type);
			const size_type allocmax = std::allocator_traits<AllocatorType>::max_size(allocator());
			return (std::min)(diffmax, allocmax);
		}

		constexpr inline size_type capacity() const { return m_end - m_start; }

		constexpr inline void clear()
		{
			std::destroy(m_start, m_finish);
			m_finish = m_start;
		}

		constexpr inline void reserve(size_type n)
		{
			auto cp = capacity();

			if (n <= cp)
				return;

			auto sz = size();

			T* new_mem = allocator().allocate(n);

			if (m_start)
			{
				try
				{
					std::uninitialized_move(m_start, m_finish, new_mem);
				}
				catch (...)
				{
					allocator().deallocate(new_mem, n);
					throw;
				}

				std::destroy(m_start, m_finish);
				allocator().deallocate(m_start, cp);
			}

			m_start  = new_mem;
			m_finish = m_start + sz;
			m_end    = m_start + n;
		}

		constexpr inline void resize(size_type n)
		{
			if (n > size())
			{
				default_append(n - size());
			}
			else if (n < size())
			{
				erase_at_end(m_start + n);
			}
		}

		constexpr inline void resize(size_type n, const value_type& v)
		{
			if (n > size())
			{
				fill_insert(end(), n - size(), v);
			}
			else if (n < size())
			{
				erase_at_end(m_start + n);
			}
		}

		constexpr void assign(size_type n, const value_type& value)
		{
			if (n > capacity())
			{
				Vector tmp(n, value);
				swap(tmp);
			}
			else if (n > size())
			{
				std::fill(begin(), end(), value);
				const size_type add = n - size();
				m_finish            = std::uninitialized_fill_n(m_finish, add, value);
			}
			else
				erase_at_end(std::fill_n(m_start, n, value));
		}

		template<class InputIterator, typename = RequireInputIter<InputIterator>>
		constexpr void assign(InputIterator first, InputIterator last)
		{
			assign_aux(first, last, IteratorCategory<InputIterator>());
		}

		constexpr void assign(std::initializer_list<T> list) { assign(list.begin(), list.end()); }

		template<class... Args>
		constexpr iterator emplace(const_iterator pos, Args&&... args)
		{
			return emplace_aux(pos, std::forward<Args>(args)...);
		}

		constexpr iterator insert(const_iterator pos, const value_type& v) { return emplace(pos, v); }

		constexpr iterator insert(const_iterator pos, value_type&& v) { return emplace(pos, std::move(v)); }

		constexpr iterator insert(const_iterator pos, size_type n, const value_type& v)
		{
			difference_type offset = pos - cbegin();
			fill_insert(begin() + offset, n, v);
			return begin() + offset;
		}

		template<class InputIterator, typename = RequireInputIter<InputIterator>>
		constexpr iterator insert(const_iterator pos, InputIterator first, InputIterator last)
		{
			difference_type offset = pos - cbegin();
			range_insert(begin() + offset, first, last, IteratorCategory<InputIterator>());
			return begin() + offset;
		}

		constexpr iterator insert(const_iterator pos, const std::initializer_list<value_type>& v)
		{
			return insert(pos, v.begin(), v.end());
		}

		template<typename... Args>
		constexpr reference emplace_back(Args&&... args)
		{
			if (m_finish != m_end)
			{
				construct_at(m_finish, std::forward<Args>(args)...);
				++m_finish;
			}
			else
			{
				realloc_insert(end(), std::forward<Args>(args)...);
			}
			return back();
		}

		constexpr reference push_back(value_type&& v) { return emplace_back(std::move(v)); }

		constexpr reference push_back(const value_type& v) { return emplace_back(v); }

		constexpr void pop_back()
		{
			if (!empty())
			{
				--m_finish;
				std::destroy_at(m_finish);
			}
		}

		constexpr iterator erase(const_iterator pos)
		{
			if (empty())
				return const_cast<iterator>(pos);

			iterator non_const_pos = const_cast<iterator>(pos);
			if (pos + 1 != end())
				std::move(non_const_pos + 1, end(), non_const_pos);
			--m_finish;
			std::destroy_at(m_finish);
			return non_const_pos;
		}

		constexpr iterator erase(const_iterator first, const_iterator last)
		{
			if (first != last)
			{
				iterator non_const_first = const_cast<iterator>(first);
				iterator non_const_last  = const_cast<iterator>(last);

				if (non_const_first != end())
					std::move(non_const_last, end(), non_const_first);

				erase_at_end(non_const_first + (end() - non_const_last));
			}
			return const_cast<iterator>(first);
		}

		constexpr void shrink_to_fit()
		{
			if (capacity() != size())
			{
				Vector copy = *this;
				swap(copy);
			}
		}

		constexpr void swap(Vector<T, AllocatorType>& other)
		{
			std::swap(m_start, other.m_start);
			std::swap(m_finish, other.m_finish);
			std::swap(m_end, other.m_end);
		}
	};
}// namespace Engine::Containers


namespace Engine
{
	template<typename Type>
	using Vector = Containers::Vector<Type, Allocator<Type>>;
	using Buffer = Vector<unsigned char>;

	template<typename Type, typename ArchiveType>
	inline bool trinex_serialize_vector(ArchiveType& ar, Vector<Type>& vector)
	    requires(is_complete_archive_type<ArchiveType>)
	{
		return ar.serialize_vector(vector);
	}

	template<typename Type, typename Alloc>
	struct Serializer<Containers::Vector<Type, Alloc>> {
		bool serialize(Archive& ar, Containers::Vector<Type, Alloc>& vector) { return trinex_serialize_vector(ar, vector); }
	};
}// namespace Engine
