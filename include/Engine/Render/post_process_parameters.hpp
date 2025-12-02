#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	struct PostProcessParameters final {
		trinex_declare_struct(PostProcessParameters, void);

		struct SSAO {
			trinex_declare_struct(SSAO, void);

			bool enabled    = true;
			float intensity = 1.f;
			float bias      = 0.03f;
			float power     = 2.f;
			float radius    = 0.01f;

			float fade_out_distance = 80.f;
			float fade_out_radius   = 50.f;
			uint_t samples          = 32;
		};

		struct Bloom {
			trinex_declare_struct(Bloom, void);

			bool enabled    = true;
			float intensity = 1.f;
			float threshold = 1.f;
			float knee      = 0.5f;
			float clamp     = 3.f;
			float fade_base = 1.f;
			float fade_max  = 1.f;
		};

		SSAO ssao;
		Bloom bloom;

		PostProcessParameters& blend(const PostProcessParameters& parameters, float blend_factor);
	};
}// namespace Engine
