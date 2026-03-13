#pragma once
#include <Core/engine_types.hpp>

namespace Trinex
{
	struct MaterialDomain {
		enum Enum : EnumerateType
		{
			Surface     = 0,
			PostProcess = 1,
		};

		trinex_enum_struct(MaterialDomain);
		trinex_enum(MaterialDomain);
	};

	struct MaterialDepthMode {
		enum Enum : EnumerateType
		{
			Disabled     = 0,
			Test         = BIT(0),
			Write        = BIT(1),
			TestAndWrite = Test | Write,
		};

		trinex_bitfield_enum_struct(MaterialDepthMode, EnumerateType);
		trinex_enum(MaterialDepthMode);

		inline bool is_test_enabled() const { return bitfield & Test; }
		inline bool is_write_enabled() const { return bitfield & Write; }
	};

	struct MaterialBlendMode {
		enum Enum : EnumerateType
		{
			Opaque      = 0,
			Masked      = 1,
			Translucent = 2,
			Additive    = 3,
			Modulate    = 4,
		};

		trinex_enum_struct(MaterialBlendMode);
		trinex_enum(MaterialBlendMode);

		inline bool is_opaque() const { return value == Opaque; }
		inline bool is_masked() const { return value == Masked; }
		inline bool is_translucent() const { return value == Translucent; }
		inline bool is_additive() const { return value == Additive; }
		inline bool is_modulate() const { return value == Modulate; }
	};
}// namespace Trinex
