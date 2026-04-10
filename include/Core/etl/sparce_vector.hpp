#pragma once
#include <Core/etl/vector.hpp>

namespace Trinex
{
	template<typename T, typename Index = u64, typename AllocatorType = Allocator<T>>
	class SparceVector
	{
	public:
		static_assert(sizeof(T) >= sizeof(Index));
		static_assert(alignof(T) >= alignof(Index));
		static_assert(std::is_trivially_destructible_v<T> && std::is_standard_layout_v<T>);

		using Container       = Vector<T, AllocatorType>;
		using value_type      = T;
		using reference       = T&;
		using const_reference = const T&;
		using pointer         = T*;
		using const_pointer   = const T*;

		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;

	private:
		Container m_container;
		Index m_free_head = ~Index(0);

	private:
		Index& next_free(Index idx) noexcept { return *reinterpret_cast<Index*>(&m_container[idx]); }
		const Index& next_free(Index idx) const noexcept { return *reinterpret_cast<const Index*>(&m_container[idx]); }

	public:
		SparceVector(size_type size = 0) { m_container.reserve(size); }

		Index emplace()
		{
			if (m_free_head != ~Index{0})
			{
				Index idx   = m_free_head;
				m_free_head = next_free(idx);

				new (&m_container[idx]) T();
				return idx;
			}

			Index idx = static_cast<Index>(m_container.size());
			m_container.emplace_back();
			return idx;
		}

		template<typename... Args>
		Index emplace(Args&&... args)
		{
			if (m_free_head != ~Index{0})
			{
				Index idx   = m_free_head;
				m_free_head = next_free(idx);
				new (&m_container[idx]) T(std::forward<Args>(args)...);
				return idx;
			}

			Index idx = static_cast<Index>(m_container.size());
			m_container.emplace_back(std::forward<Args>(args)...);
			return idx;
		}

		void erase(Index idx)
		{
			trinex_assert(idx < m_container.size());
			std::destroy_at(m_container.data() + idx);

			next_free(idx) = m_free_head;
			m_free_head    = idx;
		}

		const Container& container() const { return m_container; }
		const VectorStorage& storage() const { return m_container.storage(); }

		constexpr pointer data() noexcept { return m_container.data(); }
		constexpr const_pointer data() const noexcept { return m_container.data(); }

		constexpr reference operator[](size_type n) { return m_container[n]; }
		constexpr const_reference operator[](size_type n) const { return m_container[n]; }

		constexpr reference at(size_type n) { return (*this)[n]; }
		constexpr const_reference at(size_type n) const { return (*this)[n]; }
	};
}// namespace Trinex
