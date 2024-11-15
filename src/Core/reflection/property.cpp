#include <Core/archive.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>
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
	implement_reflect_type(MatrixProperty);
	implement_reflect_type(StringProperty);
	implement_reflect_type(NameProperty);
	implement_reflect_type(PathProperty);
	implement_reflect_type(ObjectProperty);
	implement_reflect_type(StructProperty);
	implement_reflect_type(ArrayProperty);

	void Property::trigger_object_event(const PropertyChangedEvent& event)
	{
		Engine::Object* object = reinterpret_cast<Engine::Object*>(event.context);
		object->on_property_changed(event);
	}

	Property::Property(BitMask flags) : m_flags(flags)
	{
		display_name(Strings::make_sentence(name().to_string()));
	}

	bool Property::is_read_only() const
	{
		return (m_flags & IsReadOnly) == IsReadOnly;
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

	Identifier Property::add_change_listener(const ChangeListener& listener)
	{
		return m_change_listeners.push(listener);
	}

	Property& Property::push_change_listener(const ChangeListener& listener)
	{
		m_change_listeners.push(listener);
		return *this;
	}

	Property& Property::remove_change_listener(Identifier id)
	{
		m_change_listeners.remove(id);
		return *this;
	}

	Property& Property::on_property_changed(const PropertyChangedEvent& event)
	{
		m_change_listeners(event);
		return *this;
	}

	const ScriptFunction& Property::renderer() const
	{
		auto* meta = find_metadata(Meta::renderer);
		if (meta && meta->is_a<ScriptFunction>())
		{
			return meta->cast<const ScriptFunction&>();
		}

		return default_value_of<ScriptFunction>();
	}

	Property& Property::renderer(const ScriptFunction& func)
	{
		metadata(Meta::renderer, func);
		return *this;
	}

	Property::~Property()
	{}

	bool PrimitiveProperty::serialize(void* object, Archive& ar)
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

	size_t BooleanProperty::size() const
	{
		return sizeof(bool);
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
		String element_name = element_property()->script_type_name();

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

	String MatrixProperty::script_type_name() const
	{
		return "";
	}

	EnumProperty::EnumProperty(Enum* enum_instance, BitMask flags) : Super(flags), m_enum(enum_instance)
	{}

	Enum* EnumProperty::enum_instance() const
	{
		return m_enum;
	}

	EnumProperty& EnumProperty::bind_enum(Enum* instance)
	{
		m_enum = instance;
		return *this;
	}

	String EnumProperty::script_type_name() const
	{
		return m_enum->full_name();
	}

	EnumerateType EnumProperty::value(const void* context) const
	{
		EnumerateType result = 0;
		std::memcpy(&result, address(context), size());
		return result;
	}

	EnumProperty& EnumProperty::value(void* context, EnumerateType value)
	{
		std::memcpy(address(context), &value, size());
		return *this;
	}

	bool StringProperty::serialize(void* object, Archive& ar)
	{
		String& value = *address_as<String>(object);
		return (ar & value);
	}

	String StringProperty::script_type_name() const
	{
		return "string";
	}

	size_t StringProperty::size() const
	{
		return sizeof(String);
	}

	bool NameProperty::serialize(void* object, Archive& ar)
	{
		Name& value = *address_as<Name>(object);
		return (ar & value);
	}

	String NameProperty::script_type_name() const
	{
		return "Engine::Name";
	}

	bool PathProperty::serialize(void* object, Archive& ar)
	{
		Path& value = *address_as<Path>(object);
		return ar & value;
	}

	String PathProperty::script_type_name() const
	{
		return "";
	}

	size_t ObjectProperty::size() const
	{
		return sizeof(Engine::Object*);
	}

	bool ObjectProperty::serialize(void* object, Archive& ar)
	{
		Engine::Object*& instance = *address_as<Engine::Object*>(object);

		if (m_is_composite)
		{
			return instance->serialize(ar);
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

	bool ObjectProperty::is_composite() const
	{
		return m_is_composite;
	}

	ObjectProperty& ObjectProperty::is_composite(bool flag)
	{
		m_is_composite = flag;
		return *this;
	}

	bool StructProperty::serialize(void* object, Archive& ar)
	{
		return struct_instance()->serialize(address(object), ar);
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

	bool ArrayProperty::serialize(void* object, Archive& ar)
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
			void* element = at(object, i);
			element_prop->serialize(element, ar);
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

	void Property::register_layout(ScriptClassRegistrar& r, ClassInfo* info, DownCast downcast)
	{
		Super::register_layout(r, info, downcast);
	}

	static void on_init()
	{
		{
			using T = Property::Flag;

			ScriptEnumRegistrar r("Engine::Refl::Property::Flag");
			r.set("property", 0);
			r.set("is_read_only", T::IsReadOnly);
			r.set("is_not_serializable", T::IsNotSerializable);
			r.set("is_hidden", T::IsHidden);
			r.set("is_color", T::IsColor);
		}

		ScriptClassRegistrar::RefInfo info;
		info.implicit_handle = true;
		info.no_count        = true;

		auto r = ScriptClassRegistrar::reference_class("Engine::Refl::Property", info);
		Property::register_layout(r, Property::static_refl_class_info(), script_downcast<Property>);
	}

	static ReflectionInitializeController initializer(on_init, "Engine::Refl::Property");
}// namespace Engine::Refl
