#pragma once

namespace Trinex
{
	template<typename T>
	class ScopedValue
	{
		T& m_value;
		T m_previous;

		static void swap(T& a, T& b) noexcept
		{
			T tmp(static_cast<T&&>(a));
			a = static_cast<T&&>(b);
			b = static_cast<T&&>(tmp);
		}

	public:
		template<typename Arg>
		ScopedValue(T& value, Arg&& arg) : m_value(value), m_previous(static_cast<Arg&&>(arg))
		{
			static_assert(noexcept(T(static_cast<T&&>(value))) && noexcept(value = static_cast<T&&>(m_previous)),
			              "T must support noexcept move");

			swap(m_value, m_previous);
		}

		~ScopedValue() noexcept { swap(m_value, m_previous); }

		ScopedValue(const ScopedValue&)            = delete;
		ScopedValue& operator=(const ScopedValue&) = delete;
	};

} // namespace Trinex
