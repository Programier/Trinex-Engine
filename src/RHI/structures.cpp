#include <Core/archive.hpp>
#include <Core/reflection/deprecated/property.hpp>
#include <Core/reflection/deprecated/struct.hpp>
#include <RHI/structures.hpp>

namespace Trinex
{
	bool RHIShaderParameterInfo::serialize(Archive& ar)
	{
		ar.serialize(name);
		ar.serialize(size);
		ar.serialize(offset);
		ar.serialize(type);
		ar.serialize(semantic);
		ar.serialize(binding);
		return ar;
	}

	bool RHIInputAttribute::serialize(Archive& ar)
	{
		ar.serialize(semantic);
		return ar.serialize(binding);
	}
} // namespace Trinex
