#include <Core/math/math.hpp>
#include <Core/reflection/deprecated/enum.hpp>
#include <RHI/enums.hpp>

namespace Trinex
{
	static RHIColorFormat::Capabilities s_capabilities[RHIColorFormat::static_count()] = {};

	RHIColorFormat::Capabilities RHIColorFormat::capabilities() const
	{
		return s_capabilities[value];
	}

	RHIColorFormat RHIColorFormat::add_capabilities(Capabilities capabilities) const
	{
		s_capabilities[value] |= capabilities;
		return *this;
	}

	RHIColorFormat RHIColorFormat::remove_capabilities(Capabilities capabilities) const
	{
		s_capabilities[value] &= ~capabilities;
		return *this;
	}

	trinex_api RHIShaderParameterType RHIShaderParameterType::make_numeric(RHIShaderParameterType base, u8 len)
	{
		base = base.element_type();

		if (base == Undefined)
			return Undefined;

		base &= RHIShaderParameterType(~META_DimensionMask);

		const EnumerateType size = Math::clamp<EnumerateType>(len, 1, 4);
		return base | (size << 17) | (size << 20);
	}

	trinex_api RHIShaderParameterType RHIShaderParameterType::make_matrix(RHIShaderParameterType base, u8 rows, u8 columns)
	{
		base = base.element_type();

		if (base == Undefined)
			return Undefined;

		base &= RHIShaderParameterType(~META_DimensionMask);
		return (Math::clamp<EnumerateType>(rows, 1, 4) << 17) | (Math::clamp<EnumerateType>(columns, 1, 4) << 20);
	}

	RHIShaderParameterType RHIShaderParameterType::element_type()
	{
		RHIShaderParameterType result = value & META_ElementMask;

		if (result != Undefined)
		{
			if (result == META_Unsigned)
				result |= META_Integer;

			return result | META_Concrete | META_Numeric | META_UniformBuffer | META_1D;
		}

		return Undefined;
	}
} // namespace Trinex
