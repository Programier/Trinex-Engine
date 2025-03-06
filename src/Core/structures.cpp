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

	bool ShaderParameterInfo::serialize(Archive& ar)
	{
		ar.serialize(type);
		ar.serialize(name);
		ar.serialize(size);
		ar.serialize(offset);
		ar.serialize(location);
		return ar;
	}

	trinex_implement_struct(Engine::ShaderDefinition, 0)
	{
		Refl::Struct* self = static_struct_instance();
		trinex_refl_prop(self, This, key)->display_name("Key").tooltip("Key of definition");
		trinex_refl_prop(self, This, value)->display_name("Value").tooltip("Value of definition");
	}
}// namespace Engine
