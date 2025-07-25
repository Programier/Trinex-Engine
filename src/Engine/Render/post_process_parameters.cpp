#include <Engine/Render/post_process_parameters.hpp>

namespace Engine
{
	PostProcessParameters& PostProcessParameters::blend(const PostProcessParameters& parameters, float blend_factor)
	{
		return *this;
	}
}// namespace Engine
