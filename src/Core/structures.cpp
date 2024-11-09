#include <Core/archive.hpp>
#include <Core/constants.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <Core/structures.hpp>

namespace Engine
{
	const BindLocation BindLocation::undefined = BindLocation();

	bool ShaderDefinition::serialize(class Archive& ar)
	{
		ar & key;
		ar & value;
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
		trinex_refl_prop(self, This, key)->display_name("Key").tooltip("Key of definition");
		trinex_refl_prop(self, This, value)->display_name("Value").tooltip("Value of definition");
	}
}// namespace Engine
