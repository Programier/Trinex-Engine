#pragma once
#include <Core/types/color.hpp>

namespace Engine
{
	struct LightRenderParameters {
		alignas(16) LinearColor color;
		alignas(16) Vector3f location;
		alignas(16) Vector3f direction;
		alignas(8) Vector2f spot_angles;
		alignas(4) float intensivity;
		alignas(4) float attenuation_radius;
		alignas(4) float inv_attenuation_radius;
		alignas(4) float fall_off_exponent;
		alignas(4) float source_radius;
		alignas(4) float depth_bias;
		alignas(4) float slope_scale;
	};

	struct LightRenderRanges {
		struct alignas(8) Range {
			uint_t start = 0;
			uint_t end   = 0;
		};

		Range point;
		Range spot;
		Range directional;
	};
}// namespace Engine
