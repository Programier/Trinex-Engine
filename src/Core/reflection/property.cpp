#include <Core/archive.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <ScriptEngine/script_type_info.hpp>

namespace Engine::Refl
{
	implement_reflect_type(Property);
	implement_reflect_type(PrimitiveProperty);
	implement_reflect_type(BooleanProperty);
	implement_reflect_type(IntegerProperty);
	implement_reflect_type(FloatProperty);
	implement_reflect_type(EnumProperty);
	implement_reflect_type(VectorProperty);
	implement_reflect_type(StringProperty);
	implement_reflect_type(NameProperty);
	implement_reflect_type(PathProperty);
	implement_reflect_type(ObjectProperty);
	implement_reflect_type(StructProperty);
	implement_reflect_type(ArrayProperty);

	Property::Property(BitMask flags) : m_flags(flags)
	{}

	bool Property::is_const() const
	{
		return (m_flags & IsConst) == IsConst;
	}

	bool Property::is_private() const
	{
		return (m_flags & IsPrivate) == IsPrivate;
	}

	bool Property::is_serializable() const
	{
		return (m_flags & IsNotSerializable) != IsNotSerializable;
	}

	bool Property::is_hidden() const
	{
		return (m_flags & IsHidden) == IsHidden;
	}

	bool Property::is_color() const
	{
		return (m_flags & IsColor) == IsColor;
	}

	Property::~Property()
	{}

	bool PrimitiveProperty::archive_process(void* object, Archive& ar)
	{
		byte* prop = reinterpret_cast<byte*>(address(object));

		if (ar.is_reading())
		{
			ar.read_data(prop, size());
		}
		else if (ar.is_saving())
		{
			ar.write_data(prop, size());
		}

		return ar;
	}

	String BooleanProperty::script_type_name() const
	{
		return "bool";
	}

	String IntegerProperty::script_type_name() const
	{
		return Strings::format("{}{}", is_signed() ? "int" : "uint", size() * 8);
	}

	bool IntegerProperty::is_unsigned() const
	{
		return !is_signed();
	}

	String FloatProperty::script_type_name() const
	{
		if (size() == sizeof(float))
			return "float";
		return "double";
	}

	String VectorProperty::script_type_name() const
	{
		String element_name = element_property(0)->script_type_name();

		if (length() == 1)
			return element_name;

		String vector_name = "Engine::";

		if (element_name == "bool")
		{
			vector_name += "BoolVector";
		}
		else if (element_name == "int32")
		{
			vector_name += "IntVector";
		}
		else if (element_name == "uint32")
		{
			vector_name += "UIntVector";
		}
		else if (element_name == "float")
		{
			vector_name += "Vector";
		}
		else
		{
			return "";
		}

		return Strings::format("{}{}D", vector_name, length());
	}

	EnumProperty::EnumProperty(Enum* enum_instance, BitMask flags) : Super(flags), m_enum(enum_instance)
	{}

	Enum* EnumProperty::enum_instance() const
	{
		return m_enum;
	}

	String EnumProperty::script_type_name() const
	{
		return m_enum->full_name();
	}


	bool StringProperty::archive_process(void* object, Archive& ar)
	{
		String& value = *address_as<String>(object);
		return (ar & value);
	}

	String StringProperty::script_type_name() const
	{
		return "string";
	}

	bool NameProperty::archive_process(void* object, Archive& ar)
	{
		Name& value = *address_as<Name>(object);
		return (ar & value);
	}

	String NameProperty::script_type_name() const
	{
		return "Engine::Name";
	}

	bool PathProperty::archive_process(void* object, Archive& ar)
	{
		Path& value = *address_as<Path>(object);
		return ar & value;
	}

	String PathProperty::script_type_name() const
	{
		return "";
	}

	bool ObjectProperty::archive_process(void* object, Archive& ar)
	{
		Engine::Object*& instance = *address_as<Engine::Object*>(object);

		if (m_inline_serialization)
		{
			return instance->archive_process(ar);
		}
		else
		{
			return ar.serialize_reference(instance);
		}
	}

	String ObjectProperty::script_type_name() const
	{
		auto& ti = class_instance()->script_type_info;

		if (ti.is_valid())
		{
			return Strings::concat_scoped_name(ti.namespace_name(), ti.name()) + "@";
		}

		return "";
	}

	Engine::Object* ObjectProperty::object(void* context)
	{
		return *address_as<Engine::Object*>(context);
	}

	const Engine::Object* ObjectProperty::object(const void* context) const
	{
		return *address_as<Engine::Object*>(context);
	}

	bool ObjectProperty::inline_serialization() const
	{
		return m_inline_serialization;
	}

	ObjectProperty& ObjectProperty::inline_serialization(bool flag)
	{
		m_inline_serialization = flag;
		return *this;
	}

	bool StructProperty::archive_process(void* object, Archive& ar)
	{
		return struct_instance()->archive_process(object, ar);
	}

	String StructProperty::script_type_name() const
	{
		auto& ti = struct_instance()->script_type_info;

		if (ti.is_valid() && ti.is_value())
		{
			return Strings::concat_scoped_name(ti.namespace_name(), ti.name());
		}

		return "";
	}

	bool ArrayProperty::archive_process(void* object, Archive& ar)
	{
		size_t elements = length(object);
		ar & elements;

		if (ar.is_reading())
		{
			resize(object, elements);
		}

		auto element_prop = element_property();

		for (size_t i = 0; i < elements; ++i)
		{
			void* element = element_address(object, i);
			element_prop->archive_process(element, ar);
		}

		return ar;
	}

	String ArrayProperty::script_type_name() const
	{
		auto element_name = element_property()->script_type_name();
		if (element_name.empty())
			return "";

		return Strings::format("Engine::Vector<{}>", element_name);
	}
}// namespace Engine::Refl
