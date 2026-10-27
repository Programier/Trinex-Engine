#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <Engine/Render/post_process_parameters.hpp>

namespace Trinex
{
	PostProcessParameters& PostProcessParameters::blend(const PostProcessParameters& parameters, float blend_factor)
	{
		(*this) = parameters;
		return *this;
	}
} // namespace Trinex
