#pragma once
#include <Engine/Render/post_process_parameters.generated.hpp>

namespace Trinex
{
	trinex_reflect()
	struct PostProcessParameters final
	{
		trinex_reflection_body();

		trinex_reflect()
		struct SSAO
		{
			trinex_reflection_body();

			trinex_property()
			bool enabled = false;

			trinex_property()
			f32 intensity = 1.f;

			trinex_property()
			f32 bias = 0.03f;

			trinex_property()
			f32 power = 2.f;

			trinex_property()
			f32 radius = 0.01f;

			trinex_property()
			f32 fade_out_distance = 80.f;

			trinex_property()
			f32 fade_out_radius = 50.f;

			trinex_property()
			u32 samples = 32;
		};

		trinex_reflect()
		struct Bloom
		{
			trinex_reflection_body();

			trinex_property()
			bool enabled = true;

			trinex_property()
			f32 intensity = 1.f;

			trinex_property()
			f32 threshold = 1.f;

			trinex_property()
			f32 knee = 0.5f;

			trinex_property()
			f32 clamp = 3.f;

			trinex_property()
			f32 fade_base = 1.f;

			trinex_property()
			f32 fade_max = 1.f;
		};

		trinex_property()
		SSAO ssao;

		trinex_property()
		Bloom bloom;

		PostProcessParameters& blend(const PostProcessParameters& parameters, float blend_factor);
	};
} // namespace Trinex
