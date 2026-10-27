#include <Core/reflection/deprecated/property.hpp>
#include <Core/reflection/deprecated/struct.hpp>
#include <Engine/Render/post_process_parameters.hpp>

namespace Trinex
{
	PostProcessParameters& PostProcessParameters::blend(const PostProcessParameters& parameters, float blend_factor)
	{
		(*this) = parameters;
		return *this;
	}
} // namespace Trinex
