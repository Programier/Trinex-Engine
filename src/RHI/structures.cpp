#include <Core/archive.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <RHI/structures.hpp>

namespace Trinex
{
	bool RHIShaderParameterInfo::serialize(Archive& ar)
	{
		ar.serialize(type);
		ar.serialize(name);
		ar.serialize(size);
		ar.serialize(offset);
		ar.serialize(binding);
		return ar;
	}

	bool RHIVertexAttribute::serialize(Archive& ar)
	{
		ar.serialize(semantic);
		return ar.serialize(binding);
	}
}// namespace Trinex
