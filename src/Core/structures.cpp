#include <Core/archive.hpp>
#include <Core/constants.hpp>
#include <Core/property.hpp>
#include <Core/struct.hpp>
#include <Core/structures.hpp>

namespace Engine
{
	const BindLocation BindLocation::undefined = BindLocation();

	ENGINE_EXPORT bool operator&(class Archive& ar, ShaderDefinition& definition)
	{
		ar & definition.key;
		ar & definition.value;
		return ar;
	}

	ENGINE_EXPORT bool operator&(Archive& ar, MaterialScalarParametersInfo& info)
	{
		ar & info.m_binding_index;
		return ar;
	}

	ENGINE_EXPORT bool operator&(Archive& ar, MaterialParameterInfo& info)
	{
		ar & info.type;
		ar & info.name;
		ar & info.size;
		ar & info.offset;
		ar & info.location;
		return ar;
	}

	MaterialParameterInfo::MaterialParameterInfo()
	    : type(MaterialParameterType::Undefined), name(""), size(0), offset(Constants::offset_none), location(BindLocation())
	{}

	implement_struct(Engine, ShaderDefinition)
	{
		Struct* self = static_struct_instance();
		self->add_property(new StringProperty("Key", "Key of definition", &ShaderDefinition::key));
		self->add_property(new StringProperty("Value", "Value of definition", &ShaderDefinition::value));
	}
}// namespace Engine
