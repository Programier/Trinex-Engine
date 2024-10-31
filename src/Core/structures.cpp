#include <Core/archive.hpp>
#include <Core/constants.hpp>
#include <Core/property.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/struct.hpp>
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
		if (ar.is_reading())
		{
			Name name;
			ar & name;
			info.type = Refl::Class::static_find(name, Refl::FindFlags::IsRequired);
		}
		else
		{
			Name name;

			if (info.type)
				name = info.type->full_name();
			ar & name;
		}

		ar & info.name;
		ar & info.size;
		ar & info.offset;
		ar & info.location;
		return ar;
	}

	MaterialParameterInfo::MaterialParameterInfo()
	    : type(nullptr), name(""), size(0), offset(Constants::offset_none), location(BindLocation())
	{}

	implement_struct(Engine::ShaderDefinition, 0)
	{
		Refl::Struct* self = static_struct_instance();
		self->add_property(new ClassProperty("Key", "Key of definition", &ShaderDefinition::key));
		self->add_property(new ClassProperty("Value", "Value of definition", &ShaderDefinition::value));
	}
}// namespace Engine
