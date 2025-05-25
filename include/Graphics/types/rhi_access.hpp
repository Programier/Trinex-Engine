#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	struct RHIAccess {
		enum Enum : EnumerateType
		{
			Undefined = 0,

			// Reading states
			CPURead       = BIT(0),
			Present       = BIT(1),
			IndirectArgs  = BIT(2),
			VertexBuffer  = BIT(3),
			IndexBuffer   = BIT(4),
			UniformBuffer = BIT(5),
			SRVCompute    = BIT(6),
			SRVGraphics   = BIT(7),
			CopySrc       = BIT(8),
			ResolveSrc    = BIT(9),

			// Writing states
			CPUWrite    = BIT(10),
			UAVCompute  = BIT(11),
			UAVGraphics = BIT(12),
			CopyDst     = BIT(13),
			ResolveDst  = BIT(14),
			RTV         = BIT(15),
			DSV         = BIT(16),

			ReadableMask = CPURead | Present | IndirectArgs | VertexBuffer | IndexBuffer | UniformBuffer | SRVCompute |
			               SRVGraphics | CopySrc | ResolveSrc | UAVCompute | UAVGraphics | RTV | DSV,

			WritableMask = CPUWrite | UAVCompute | UAVGraphics | CopyDst | ResolveDst | RTV | DSV,
		};

		trinex_bitfield_enum_struct(RHIAccess, EnumerateType);
	};
};// namespace Engine
