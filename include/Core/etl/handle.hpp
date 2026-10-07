#pragma once
#include <Core/etl/storage.hpp>

namespace Trinex
{
	class ENGINE_EXPORT HandleBase
	{
	protected:
		static u64 generate();
	};

	template<typename Tag>
	class Handle : public HandleBase
	{
	private:
		u64 m_handle = 0;

	public:
		constexpr Handle() noexcept = default;

		constexpr explicit Handle(u64 handle) noexcept : m_handle(handle) {}

		[[nodiscard]]
		static Handle allocate()
		{
			return Handle(generate());
		}

		[[nodiscard]]
		constexpr u64 value() const noexcept
		{
			return m_handle;
		}

		[[nodiscard]]
		constexpr bool is_valid() const noexcept
		{
			return m_handle != 0;
		}

		constexpr explicit operator bool() const noexcept { return is_valid(); }
		constexpr void reset() noexcept { m_handle = 0; }

		constexpr bool operator==(const Handle&) const noexcept = default;
		constexpr bool operator<(const Handle& other) const noexcept { return m_handle < other.m_handle; }
		constexpr bool operator<=(const Handle& other) const noexcept { return m_handle <= other.m_handle; }
		constexpr bool operator>(const Handle& other) const noexcept { return m_handle > other.m_handle; }
		constexpr bool operator>=(const Handle& other) const noexcept { return m_handle >= other.m_handle; }
	};
}// namespace Trinex
