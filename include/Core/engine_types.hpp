#pragma once
#include <Core/definitions.hpp>
#include <Core/serializer.hpp>
#include <cstdint>

namespace Engine
{
	using byte  = std::uint8_t;
	using word  = std::uint16_t;
	using dword = std::uint32_t;
	using qword = std::uint64_t;

	using bool_t    = bool;
	using short_t   = std::int16_t;
	using ushort_t  = std::uint16_t;
	using int_t     = std::int32_t;
	using uint_t    = std::uint32_t;
	using int8_t    = std::int8_t;
	using uint8_t   = std::uint8_t;
	using int16_t   = std::int16_t;
	using uint16_t  = std::uint16_t;
	using int32_t   = std::int32_t;
	using uint32_t  = std::uint32_t;
	using int64_t   = std::int64_t;
	using uint64_t  = std::uint64_t;
	using int128_t  = signed __int128;
	using uint128_t = unsigned __int128;
	using float_t   = float;
	using double_t  = double;

	using signed_byte = std::int8_t;
	using size_t      = std::uint64_t;
	using ptrdiff_t   = std::int64_t;

	using ArrayIndex          = size_t;
	using ArrayOffset         = size_t;
	using PriorityIndex       = size_t;
	using Counter             = size_t;
	using Index               = size_t;
	using MaterialLayoutIndex = size_t;

	using BindingIndex = byte;

	using BitMask       = size_t;
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
}// namespace Engine
