#pragma once
#include <Core/engine_types.hpp>

namespace Trinex
{
	trinex_reflect()
	trinex_enum MaterialDomain
	{
		enum Enum : EnumerateType
		{
			Surface     = 0,
			PostProcess = 1,
		};

		trinex_enum_struct(MaterialDomain);
	};

	trinex_reflect()
	trinex_enum MaterialDepthMode
	{
		enum Enum : EnumerateType
		{
			Disabled     = 0,
			Test         = trinex_bit(0),
			Write        = trinex_bit(1),
			TestAndWrite = Test | Write,
		};

		trinex_bitfield_enum_struct(MaterialDepthMode, EnumerateType);

		inline bool is_test_enabled() const
		{
			return bitfield & Test;
		}

		inline bool is_write_enabled() const
		{
			return bitfield & Write;
		}
	};

	trinex_reflect()
	trinex_enum MaterialBlendMode
	{
		enum Enum : EnumerateType
		{
			Opaque      = 0,
			Masked      = 1,
			Translucent = 2,
			Additive    = 3,
			Modulate    = 4,
		};

		trinex_enum_struct(MaterialBlendMode);

		inline bool is_opaque() const
		{
			return value == Opaque;
		}

		inline bool is_masked() const
		{
			return value == Masked;
		}

		inline bool is_translucent() const
		{
			return value == Translucent;
		}

		inline bool is_additive() const
		{
			return value == Additive;
		}

		inline bool is_modulate() const
		{
			return value == Modulate;
		}
	};
} // namespace Trinex
