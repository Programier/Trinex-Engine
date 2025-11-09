#pragma once
#include <Core/engine_types.hpp>

namespace Engine
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
		trinex_declare_enum(ShowFlags);
	};

	struct ENGINE_EXPORT ViewMode {
		enum Enum : EnumerateType
		{
			Lit         = 0,
			Unlit       = 1,
			Wireframe   = 2,
			WorldNormal = 3,
			Metalic     = 4,
			Roughness   = 5,
			Specular    = 6,
			Emissive    = 7,
			AO          = 8,
			Velocity    = 9,
		};

		trinex_enum_struct(ViewMode);
		trinex_declare_enum(ViewMode);
	};

	struct ENGINE_EXPORT CameraProjectionMode {
		enum Enum : EnumerateType
		{
			Perspective  = 0,
			Orthographic = 1,
		};

		trinex_enum_struct(CameraProjectionMode);
		trinex_declare_enum(CameraProjectionMode);
	};

}// namespace Engine
