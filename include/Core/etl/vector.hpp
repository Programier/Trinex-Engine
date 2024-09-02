#pragma once
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>

namespace Engine::Containers
{
	template<typename T, typename AllocatorType = std::allocator<T>>
	class Vector
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


	private:
		pointer m_start;
		pointer m_finish;
		pointer m_end;

		template<typename IteratorType>
		using RequireInputIter =
		        std::enable_if_t<std::is_convertible<typename std::iterator_traits<IteratorType>::iterator_category,
		                                             std::input_iterator_tag>::value>;

		template<typename IteratorType>
		static constexpr inline bool is_forward_iterator =
		        std::is_convertible_v<typename std::iterator_traits<IteratorType>::iterator_category, std::forward_iterator_tag>;

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

		constexpr inline void grow(size_type n = 1)
		{
			size_type c = capacity();
			c           = (c == 0) ? 1 : c * 2;
			while (c < size() + n) c *= 2;
			reserve(c);
		}

		constexpr inline void append_to_end(size_type count)
		{
			pointer new_finish = m_finish + count;

			while (m_finish != new_finish)
			{
				std::construct_at(m_finish);
				++m_finish;
			}
		}

		constexpr inline void append_to_end(size_type count, const value_type& v)
		{
			pointer new_finish = m_finish + count;

			while (m_finish != new_finish)
			{
				std::construct_at(m_finish, v);
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
				AllocatorType().deallocate(m_start, capacity());
			}
		}

	public:
		constexpr inline iterator begin()
		{
			return m_start;
		}

		constexpr inline const_iterator begin() const
		{
			return m_start;
		}

		constexpr inline iterator end()
		{
			return m_finish;
		}

		constexpr inline const_iterator end() const
		{
			return m_finish;
		}

		constexpr inline const_iterator cbegin() const
		{
			return m_start;
		}

		constexpr inline const_iterator cend() const
		{
			return m_finish;
		}

		constexpr inline reverse_iterator rbegin()
		{
			return reverse_iterator(end());
		}

		constexpr inline const_reverse_iterator rbegin() const
		{
			return const_reverse_iterator(end());
		}

		constexpr inline reverse_iterator rend()
		{
			return reverse_iterator(begin());
		}

		constexpr inline const_reverse_iterator rend() const
		{
			return const_reverse_iterator(begin());
		}

		constexpr inline const_reverse_iterator crbegin() const
		{
			return const_reverse_iterator(end());
		}

		constexpr inline const_reverse_iterator crend() const
		{
			return const_reverse_iterator(begin());
		}

		constexpr Vector() noexcept : m_start(nullptr), m_finish(nullptr), m_end(nullptr)
		{}

		constexpr explicit Vector(size_type n) : Vector()
		{
			reserve(n);
			append_to_end(n);
		}

		constexpr Vector(size_type n, const value_type& v) : Vector()
		{
			reserve(n);
			append_to_end(n, v);
		}

		template<class InputIterator, typename = RequireInputIter<InputIterator>>
		constexpr Vector(InputIterator first, InputIterator last) : Vector()
		{
			range_initialize(first, last);
		}

		constexpr Vector(std::initializer_list<T> list) : Vector(list.begin(), list.end())
		{}

		constexpr Vector(const Vector& other) : Vector(other.begin(), other.end())
		{}

		constexpr Vector(Vector&& other) : m_start(other.m_start), m_finish(other.m_finish), m_end(other.m_end)
		{
			other.m_start = other.m_finish = other.m_end = nullptr;
		}

		constexpr ~Vector()
		{
			destroy();
		}

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

		constexpr reference operator[](size_type n)
		{
			return *(m_start + n);
		}

		constexpr const_reference operator[](size_type n) const
		{
			return *(m_start + n);
		}

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

		constexpr reference front()
		{
			return *begin();
		}

		constexpr const_reference front() const
		{
			return *begin();
		}

		constexpr reference back()
		{
			return *(end() - 1);
		}

		constexpr const_reference back() const
		{
			return *(end() - 1);
		}

		constexpr pointer data() noexcept
		{
			return data_ptr(m_start);
		}

		constexpr const_pointer data() const noexcept
		{
			return data_ptr(m_start);
		}

		constexpr inline bool empty() const
		{
			return m_start == m_finish;
		}

		constexpr inline size_type size() const
		{
			return m_finish - m_start;
		}

		constexpr inline size_type max_size() const
		{
			const size_type diffmax  = std::numeric_limits<difference_type>::max() / sizeof(value_type);
			const size_type allocmax = std::allocator_traits<AllocatorType>::max_size(AllocatorType());
			return (std::min)(diffmax, allocmax);
		}

		constexpr inline size_type capacity() const
		{
			return m_end - m_start;
		}

		constexpr inline void clear()
		{
			std::destroy(m_start, m_finish);
			m_finish = m_start;
		}

		constexpr inline void reserve(size_type n)
		{
			auto cp = capacity();

			if (n < cp)
				return;

			auto sz = size();

			T* new_mem = AllocatorType().allocate(n);

			if (m_start)
			{
				try
				{
					std::uninitialized_move(m_start, m_finish, new_mem);
				}
				catch (...)
				{
					AllocatorType().deallocate(new_mem, n);
					throw;
				}

				std::destroy(m_start, m_finish);
				AllocatorType().deallocate(m_start, cp);
			}

			m_start  = new_mem;
			m_finish = m_start + sz;
			m_end    = m_start + n;
		}

		constexpr inline void resize(size_type n)
		{
			if (n > size())
			{
				reserve(n);
				append_to_end(n);
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
				reserve(n);
				append_to_end(n, v);
			}
			else if (n < size())
			{
				erase_at_end(m_start + n);
			}
		}

		constexpr void assign(size_type n, const value_type& value)
		{
			clear();
			reserve(n);
			append_to_end(n, value);
		}

		template<class InputIterator, typename = RequireInputIter<InputIterator>>
		constexpr void assign(InputIterator first, InputIterator last)
		{
			clear();
			range_initialize(first, last);
		}

		constexpr void assign(std::initializer_list<T> list)
		{
			assign(list.begin(), list.end());
		}

		template<class... Args>
		constexpr iterator emplace(const_iterator pos, Args&&... args)
		{
			if (m_finish == m_end)
			{
				auto offset = std::distance(cbegin(), pos);
				grow();
				pos = cbegin() + offset;
			}

			iterator non_const_pos = const_cast<iterator>(pos);

			if (non_const_pos != end())
			{
				std::move_backward(non_const_pos, end(), end() + 1);
				std::destroy_at(non_const_pos);
			}

			std::construct_at(pos, std::forward<Args>(args)...);
			++m_finish;
			return non_const_pos;
		}

		constexpr iterator insert(const_iterator pos, const value_type& v)
		{
			return emplace(pos, v);
		}

		constexpr iterator insert(const_iterator pos, value_type&& v)
		{
			return emplace(pos, std::move(v));
		}

		constexpr iterator insert(const_iterator pos, size_type n, const value_type& v)
		{
			if (n == 0)
			{
				return const_cast<iterator>(pos);
			}

			if (m_finish + n > m_end)
			{
				auto offset = std::distance(cbegin(), pos);
				grow(n);
				pos = cbegin() + offset;
			}

			iterator non_const_pos = const_cast<iterator>(pos);

			if (non_const_pos != end())
			{
				std::move_backward(non_const_pos, end(), end() + n);

				for (iterator it = end() - 1; it >= non_const_pos; --it)
				{
					std::destroy_at(it);
				}
			}

			std::uninitialized_fill_n(non_const_pos, n, v);

			m_finish += n;
			return non_const_pos;
		}

		template<class InputIterator, typename = RequireInputIter<InputIterator>>
		constexpr iterator insert(const_iterator pos, InputIterator first, InputIterator last)
		{
			if (first == last)
			{
				return const_cast<iterator>(pos);
			}

			auto n = std::distance(first, last);

			if (m_finish + n > m_end)
			{
				auto offset = std::distance(cbegin(), pos);
				grow(n);
				pos = cbegin() + offset;
			}

			iterator non_const_pos = const_cast<iterator>(pos);

			if (non_const_pos != end())
			{
				std::move_backward(non_const_pos, end(), end() + n);

				for (iterator it = end() - 1; it >= non_const_pos; --it)
				{
					std::destroy_at(it);
				}
			}

			std::uninitialized_copy(first, last, non_const_pos);
			m_finish += n;
			return non_const_pos;
		}

		constexpr iterator insert(const_iterator pos, const std::initializer_list<value_type>& v)
		{
			return insert(pos, v.begin(), v.end());
		}

		template<typename... Args>
		constexpr reference emplace_back(Args&&... args)
		{
			if (m_finish == m_end)
				grow();
			std::construct_at(m_finish, std::forward<Args>(args)...);
			++m_finish;
			return back();
		}

		constexpr reference push_back(value_type&& v)
		{
			return emplace_back(std::move(v));
		}

		constexpr reference push_back(const value_type& v)
		{
			return emplace_back(v);
		}

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

			std::destroy_at(non_const_pos);
			std::move(non_const_pos + 1, end(), non_const_pos);
			--m_finish;
			return non_const_pos;
		}

		constexpr iterator erase(const_iterator first, const_iterator last)
		{
			if (empty())
				return const_cast<iterator>(first);

			iterator non_const_first = const_cast<iterator>(first);
			iterator non_const_last  = const_cast<iterator>(last);

			std::destroy(non_const_first, non_const_last);
			std::move(non_const_last, end(), non_const_first);
			m_finish -= std::distance(first, last);
			return non_const_first;
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
