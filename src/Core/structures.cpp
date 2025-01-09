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
		ar.serialize(key);
		ar.serialize(value);
		return ar;
	}


	bool MaterialParameterInfo::serialize(Archive& ar)
	{
		if (ar.is_reading())
		{
			Name name;
			name.serialize(ar);
			type = Refl::Class::static_find(name, Refl::FindFlags::IsRequired);
		}
		else
		{
			Name name;

			if (type)
				name = type->full_name();
			name.serialize(ar);
		}

		ar.serialize(name);
		ar.serialize(size);
		ar.serialize(offset);
		ar.serialize(location);
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
