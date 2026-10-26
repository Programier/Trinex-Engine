#pragma once
#include <Core/engine_types.hpp>

namespace Trinex
{
	trinex_reflect()
	trinex_enum ShowFlags
	{
		enum Enum : EnumerateType
		{
			None              = 0,
			Statistics        = trinex_bit(1),
			PointLights       = trinex_bit(2),
			SpotLights        = trinex_bit(3),
			DirectionalLights = trinex_bit(4),
			PostProcess       = trinex_bit(5),
			StaticMesh        = trinex_bit(6),
			PrimitiveBounds   = trinex_bit(7),

			DefaultFlags = PointLights | SpotLights | DirectionalLights | StaticMesh | PostProcess,
		};

		trinex_bitfield_enum_struct(ShowFlags, EnumerateType);
	};

	trinex_reflect()
	trinex_enum ViewMode
	{
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
	};

	trinex_reflect()
	trinex_enum CameraProjectionMode
	{
		enum Enum : EnumerateType
		{
			Perspective  = 0,
			Orthographic = 1,
		};

		trinex_enum_struct(CameraProjectionMode);
	};

} // namespace Trinex
