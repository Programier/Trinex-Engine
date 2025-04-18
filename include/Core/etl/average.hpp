#pragma once
#include <type_traits>

namespace Engine
{
	template<typename Type, std::size_t max_count = ~static_cast<std::size_t>(0),
	         typename = typename std::enable_if<std::is_arithmetic<Type>::value>::type>
	class Average
	{
		std::size_t m_count = 0;
		Type m_value        = static_cast<Type>(0);

	public:
		Average(Type value = Type(0), std::size_t initial_count = 0)
		    : m_count(initial_count < max_count ? initial_count : max_count), m_value(value)
		{}

		Average(const Average&)            = default;
		Average& operator=(const Average&) = default;

		Average& push(const Type& value)
		{
			if (m_count < max_count)
				++m_count;

			if (m_count > 1)
			{
				m_value /= (static_cast<Type>(m_count) / static_cast<Type>(m_count - 1));
			}

			m_value += value / static_cast<Type>(m_count);
			return *this;
		}

		template<typename... Args>
		Average& push(const Type& value, Args... args)
		{
			push(value);
			for (auto& ell : {args...})
			{
				push(ell);
			}

			return *this;
		}

		operator Type() const { return m_value; }

		const Type& average() const { return m_value; }

		Average& reset()
		{
			m_value = static_cast<Type>(0);
			m_count = 0;
			return *this;
		}

		std::size_t count() const { return m_count; }

		template<typename OutType, std::size_t out_max_cout>
		operator Average<OutType, out_max_cout>() const
		{
			return Average<OutType, out_max_cout>(static_cast<OutType>(m_value), m_count);
		}
	};
}// namespace Engine
