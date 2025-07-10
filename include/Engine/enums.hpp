#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	struct ShowFlags {
		enum Enum : EnumerateType
		{
			None              = 0,
			Sprite            = BIT(0),
			Wireframe         = BIT(1),
			Gizmo             = BIT(2),
			PointLights       = BIT(3),
			SpotLights        = BIT(4),
			DirectionalLights = BIT(5),
			PostProcess       = BIT(6),
			StaticMesh        = BIT(7),
			LightOctree       = BIT(8),
			PrimitiveOctree   = BIT(9),
			Statistics        = BIT(10),

			DefaultFlags = Sprite | PointLights | SpotLights | DirectionalLights | StaticMesh | PostProcess,
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

}// namespace Engine
