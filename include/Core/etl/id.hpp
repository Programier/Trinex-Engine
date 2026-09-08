#pragma once

namespace Trinex::etl
{
	template<typename Mark, typename Type = u64>
	class ID
	{
	public:
		using mark_type  = Mark;
		using value_type = Type;

		constexpr ID() noexcept = default;

		explicit constexpr ID(Type value) noexcept : m_value(value) {}

		[[nodiscard]]
		constexpr Type value() const noexcept
		{
			return m_value;
		}

		[[nodiscard]]
		constexpr bool valid() const noexcept
		{
			return m_value != invalid_value;
		}

		[[nodiscard]]
		explicit constexpr operator bool() const noexcept
		{
			return valid();
		}

		[[nodiscard]]
		explicit constexpr operator Type() const noexcept
		{
			return m_value;
		}

		constexpr auto operator<=>(const ID&) const noexcept = default;

		static constexpr Type invalid_value = Type{0};

		static constexpr ID invalid() noexcept { return ID{invalid_value}; }

	private:
		Type m_value = invalid_value;
	};
}// namespace Trinex::etl
