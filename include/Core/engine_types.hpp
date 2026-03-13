#pragma once
#include <Core/serializer.hpp>
#include <cstdint>

namespace Trinex
{
	using u8   = std::uint8_t;
	using u16  = std::uint16_t;
	using u32  = std::uint32_t;
	using u64  = std::uint64_t;
	using u128 = unsigned __int128;

	using i8   = std::int8_t;
	using i16  = std::int16_t;
	using i32  = std::int32_t;
	using i64  = std::int64_t;
	using i128 = __int128;

	using f16 = _Float16;
	using f32 = float;
	using f64 = double;

	using usize = u64;
	using isize = i64;

	using BitMask       = usize;
	using Identifier    = std::uint64_t;
	using EnumerateType = std::uint32_t;

	namespace Refl
	{
		class Enum;
		class Struct;
		class Class;
		class Property;
		struct PropertyChangedEvent;
	}// namespace Refl
}// namespace Trinex
