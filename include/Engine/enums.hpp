#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	struct ShowFlags {
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

			DefaultFlags = PointLights | SpotLights | DirectionalLights | StaticMesh | PostProcess,
		};

		trinex_bitfield_enum_struct(ShowFlags, EnumerateType);
		trinex_declare_enum(ShowFlags);
	};

	struct ViewMode {
		enum Enum : EnumerateType
		{
			Lit         = 0,
			Unlit       = 1,
			Wireframe   = 2,
			WorldNormal = 3,
			Metalic     = 4,
			Roughness   = 5,
			Specular    = 6,
			AO          = 7,
		};

		trinex_enum_struct(ViewMode);
		trinex_declare_enum(ViewMode);
	};

	struct CameraProjectionMode {
		enum Enum : EnumerateType
		{
			Perspective  = 0,
			Orthographic = 1,
		};

		trinex_enum_struct(CameraProjectionMode);
		trinex_declare_enum(CameraProjectionMode);
	};

}// namespace Engine
