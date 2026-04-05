#pragma once

namespace Trinex
{
	template<typename Underlying>
	struct Flags {
		using Enum = Underlying;
		trinex_bitfield_enum_struct(Flags, Underlying);
	};
}// namespace Trinex
