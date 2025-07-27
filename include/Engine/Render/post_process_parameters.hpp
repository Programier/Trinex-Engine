#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	struct PostProcessParameters final {
		trinex_declare_struct(PostProcessParameters, void);

		struct SSAO {
			trinex_declare_struct(SSAO, void);

			bool enabled    = false;
			float intensity = 1.f;
			float bias      = 0.03f;
			float power     = 2.f;
			float radius    = 0.35f;

			float fade_out_distance = 80.f;
			float fade_out_radius   = 50.f;
			uint samples            = 32;
		};

		SSAO ssao;

		PostProcessParameters& blend(const PostProcessParameters& parameters, float blend_factor);
	};
}// namespace Engine
