#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	struct RHIAccess {
		enum Enum : EnumerateType
		{
			Undefined = 0,

			// Reading states
			CPURead             = BIT(0),
			Present             = BIT(1),
			IndirectArgs        = BIT(2),
			VertexOrIndexBuffer = BIT(3),
			SRVCompute          = BIT(4),
			SRVGraphics         = BIT(5),
			CopySrc             = BIT(6),
			ResolveSrc          = BIT(7),

			// Writing states
			UAVCompute  = BIT(8),
			UAVGraphics = BIT(9),
			CopyDst     = BIT(10),
			ResolveDst  = BIT(11),
			RTV         = BIT(12),
			DSV         = BIT(13),

			ReadableMask = CPURead | Present | IndirectArgs | VertexOrIndexBuffer | SRVCompute | SRVGraphics | CopySrc |
			               ResolveSrc | UAVCompute | UAVGraphics,

			WritableMask = UAVCompute | UAVGraphics | CopyDst | ResolveDst | RTV | DSV,
		};

		trinex_bitfield_enum_struct(RHIAccess, EnumerateType);
	};
};// namespace Engine
