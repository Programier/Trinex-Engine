#pragma once
#include <Core/engine_types.hpp>

namespace Trinex
{
	struct ENGINE_EXPORT ShowFlags {
		enum Enum : EnumerateType
		{
			None              = 0,
			Statistics        = BIT(1),
			PointLights       = BIT(2),
			SpotLights        = BIT(3),
			DirectionalLights = BIT(4),
			PostProcess       = BIT(5),
			StaticMesh        = BIT(6),
			PrimitiveBounds   = BIT(7),
			PrimitiveOctree   = BIT(8),

			DefaultFlags = PointLights | SpotLights | DirectionalLights | StaticMesh | PostProcess,
		};

		trinex_bitfield_enum_struct(ShowFlags, EnumerateType);
		trinex_enum(ShowFlags);
	};

	struct ENGINE_EXPORT ViewMode {
		enum Enum : EnumerateType
		{
			Lit         = 0,
			Unlit       = 1,
			Wireframe   = 2,
			WorldNormal = 3,
			Emissive    = 4,
			Metalic     = 5,
			Specular    = 6,
			Roughness   = 7,
			AO          = 8,
			Velocity    = 9,
			Depth       = 10,
		};

		trinex_enum_struct(ViewMode);
		trinex_enum(ViewMode);
	};

	struct ENGINE_EXPORT CameraProjectionMode {
		enum Enum : EnumerateType
		{
			Perspective  = 0,
			Orthographic = 1,
		};

		trinex_enum_struct(CameraProjectionMode);
		trinex_enum(CameraProjectionMode);
	};

}// namespace Trinex
