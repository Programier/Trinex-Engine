#include <Core/archive.hpp>
#include <RHI/structures.hpp>

namespace Engine
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
		ar.serialize(name);
		ar.serialize(type);
		ar.serialize(semantic);
		ar.serialize(semantic_index);
		return ar.serialize(binding);
	}
}// namespace Engine
