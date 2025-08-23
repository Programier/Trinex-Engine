#pragma once
#include <Core/math/matrix.hpp>
#include <Core/math/vector.hpp>
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
		alignas(4) uint32_t shadow_address;
	};

	struct LightRenderRanges {
		struct alignas(8) Range {
			uint_t start = 0;
			uint_t end   = 0;
		};

		struct alignas(8) LightRange {
			Range normal;
			Range shadowed;
		};

		LightRange point;
		LightRange spot;
		LightRange directional;
	};

	struct PointLightShadowParameters {
		uint64_t descriptor;
		float depth_bias;
		float slope_scale;
	};

	struct SpotLightShadowParameters {
		uint64_t descriptor;
		Matrix4f projview;
		float depth_bias;
		float slope_scale;
	};

	struct DirectionalLightShadowParameters {
		uint64_t descriptor;
		Matrix4f projview;
		float depth_bias;
		float slope_scale;
	};
}// namespace Engine
