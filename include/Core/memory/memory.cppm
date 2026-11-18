module;
#include <cstring>
export module Trinex.Core.Memory;
export import Trinex.Core.Types;

export namespace Trinex::Memory
{
	[[nodiscard]]
	constexpr bool is_aligned(usize value, usize alignment) noexcept
	{
		return (value & (alignment - 1)) == 0;
	}

	template<typename T>
	[[nodiscard]]
	inline bool is_aligned(const T* ptr, usize alignment) noexcept
	{
		return is_aligned(reinterpret_cast<uptr>(ptr), alignment);
	}

	[[nodiscard]]
	constexpr bool is_valid_alignment(usize alignment) noexcept
	{
		return alignment != 0 && (alignment & (alignment - 1)) == 0;
	}

	[[nodiscard]]
	constexpr usize align_down(usize value, usize alignment) noexcept
	{
		return value & ~(alignment - 1);
	}

	[[nodiscard]]
	constexpr usize align_up(usize value, usize alignment) noexcept
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}

	[[nodiscard]]
	inline void* align_down(void* ptr, usize alignment) noexcept
	{
		const auto address = reinterpret_cast<uptr>(ptr);
		return reinterpret_cast<void*>(align_down(address, alignment));
	}

	[[nodiscard]]
	inline void* align_up(void* ptr, usize alignment) noexcept
	{
		const auto address = reinterpret_cast<uptr>(ptr);
		return reinterpret_cast<void*>(align_up(address, alignment));
	}

	[[nodiscard]]
	inline const void* align_down(const void* ptr, usize alignment) noexcept
	{
		const auto address = reinterpret_cast<uptr>(ptr);
		return reinterpret_cast<const void*>(align_down(address, alignment));
	}

	[[nodiscard]]
	inline const void* align_up(const void* ptr, usize alignment) noexcept
	{
		const auto address = reinterpret_cast<uptr>(ptr);
		return reinterpret_cast<const void*>(align_up(address, alignment));
	}

	template<typename T, usize alignment = alignof(T)>
	[[nodiscard]]
	inline T* align_up(void* ptr, usize offset = 0) noexcept
	{
		static_assert(is_valid_alignment(alignment));
		static_assert(is_aligned(alignment, alignof(T)));

		const auto address = reinterpret_cast<uptr>(ptr);
		return reinterpret_cast<T*>(align_up(address + offset, alignment));
	}

	template<typename T, usize alignment = alignof(T)>
	[[nodiscard]]
	inline const T* align_up(const void* ptr, usize offset = 0) noexcept
	{
		static_assert(is_valid_alignment(alignment));
		static_assert(is_aligned(alignment, alignof(T)));

		const auto address = reinterpret_cast<uptr>(ptr);
		return reinterpret_cast<const T*>(align_up(address + offset, alignment));
	}

	template<typename T, usize alignment = alignof(T)>
	[[nodiscard]]
	inline T* align_down(void* ptr, usize offset = 0) noexcept
	{
		static_assert(is_valid_alignment(alignment));
		static_assert(is_aligned(alignment, alignof(T)));

		const auto address = reinterpret_cast<uptr>(ptr);
		return reinterpret_cast<T*>(align_down(address + offset, alignment));
	}

	template<typename T, usize alignment = alignof(T)>
	[[nodiscard]]
	inline const T* align_down(const void* ptr, usize offset = 0) noexcept
	{
		static_assert(is_valid_alignment(alignment));
		static_assert(is_aligned(alignment, alignof(T)));

		const auto address = reinterpret_cast<uptr>(ptr);
		return reinterpret_cast<const T*>(align_down(address + offset, alignment));
	}

	[[nodiscard]]
	constexpr usize align(usize value, usize alignment) noexcept
	{
		return align_up(value, alignment);
	}

	[[nodiscard]]
	inline void* align(void* ptr, usize alignment) noexcept
	{
		return align_up(ptr, alignment);
	}

	[[nodiscard]]
	inline const void* align(const void* ptr, usize alignment) noexcept
	{
		return align_up(ptr, alignment);
	}

	template<typename T, usize alignment = alignof(T)>
	[[nodiscard]]
	inline T* align(void* ptr, usize offset = 0) noexcept
	{
		return align_up<T, alignment>(ptr, offset);
	}

	template<typename T, usize alignment = alignof(T)>
	[[nodiscard]]
	inline const T* align(const void* ptr, usize offset = 0) noexcept
	{
		return align_up<T, alignment>(ptr, offset);
	}

	inline void* copy(void* dst, const void* src, usize size) noexcept
	{
		return std::memcpy(dst, src, size);
	}

	inline void* move(void* dst, const void* src, usize size) noexcept
	{
		return std::memmove(dst, src, size);
	}

	inline void* set(void* dst, u8 value, usize size) noexcept
	{
		return std::memset(dst, value, size);
	}

	[[nodiscard]]
	inline i32 compare(const void* lhs, const void* rhs, usize size) noexcept
	{
		return std::memcmp(lhs, rhs, size);
	}

	[[nodiscard]]
	inline const void* find(const void* data, usize size, u8 value) noexcept
	{
		return std::memchr(data, value, size);
	}

	[[nodiscard]]
	inline void* find(void* data, usize size, u8 value) noexcept
	{
		return std::memchr(data, value, size);
	}

	[[nodiscard]]
	inline const void* find(const void* haystack, usize haystack_len, const void* needle, usize needle_len) noexcept
	{
		if (needle_len == 0)
			return haystack;

		if (needle_len > haystack_len)
			return nullptr;

		const auto* data    = static_cast<const u8*>(haystack);
		const auto* pattern = static_cast<const u8*>(needle);

		const usize count = haystack_len - needle_len;

		for (usize i = 0; i <= count; ++i)
		{
			if (data[i] == pattern[0] && std::memcmp(data + i, pattern, needle_len) == 0)
			{
				return data + i;
			}
		}

		return nullptr;
	}

	[[nodiscard]]
	inline void* find(void* haystack, usize haystack_len, const void* needle, usize needle_len) noexcept
	{
		return const_cast<void*>(find(static_cast<const void*>(haystack), haystack_len, needle, needle_len));
	}

	[[nodiscard]]
	trinex_api u128 hash(const void* memory, usize size, u128 seed = 0);
} // namespace Trinex::Memory
