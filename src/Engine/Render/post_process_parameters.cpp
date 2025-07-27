#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <Engine/Render/post_process_parameters.hpp>

namespace Engine
{
	trinex_implement_struct(Engine::PostProcessParameters::SSAO, 0)
	{
		trinex_refl_prop(enabled);
		trinex_refl_prop(intensity);
		trinex_refl_prop(bias);
		trinex_refl_prop(power);
		trinex_refl_prop(radius);

		trinex_refl_prop(fade_out_distance);
		trinex_refl_prop(fade_out_radius);
		trinex_refl_prop(samples);
	}

	trinex_implement_struct(Engine::PostProcessParameters, 0)
	{
		trinex_refl_prop(ssao);
	}

	PostProcessParameters& PostProcessParameters::blend(const PostProcessParameters& parameters, float blend_factor)
	{
		(*this) = parameters;
		return *this;
	}
}// namespace Engine
