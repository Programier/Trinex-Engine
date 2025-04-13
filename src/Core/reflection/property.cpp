#include <Core/archive.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/logger.hpp>
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
	implement_reflect_type(ReflObjectProperty);
	implement_reflect_type(SubClassProperty);
	implement_reflect_type(FlagsProperty);

	void Property::trigger_object_event(const PropertyChangedEvent& event)
	{
		Engine::Object* object = reinterpret_cast<Engine::Object*>(event.context);
		object->on_property_changed(event);
	}

	Property::Property(BitMask flags) : m_flags(flags) {}

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

	const String& Property::property_name(const void* context)
	{
		return display_name();
	}

	Property& Property::on_property_changed(const PropertyChangedEvent& event)
	{
		m_change_listeners(event);
		return *this;
	}

	void Property::register_layout(ScriptClassRegistrar& r, ClassInfo* info, DownCast downcast)
	{
		Super::register_layout(r, info, downcast);
	}

	Property::~Property() {}

	bool PrimitiveProperty::serialize(void* object, Archive& ar)
	{
		byte* prop = reinterpret_cast<byte*>(address(object));
		ar.serialize_memory(prop, size());
		return ar;
	}

	size_t BooleanProperty::size() const
	{
		return sizeof(bool);
	}

	bool IntegerProperty::is_unsigned() const
	{
		return !is_signed();
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
		return ar.serialize(value);
	}

	size_t StringProperty::size() const
	{
		return sizeof(String);
	}

	bool NameProperty::serialize(void* object, Archive& ar)
	{
		Name& value = *address_as<Name>(object);
		return ar.serialize(value);
	}

	bool PathProperty::serialize(void* object, Archive& ar)
	{
		Path& value = *address_as<Path>(object);
		return ar.serialize(value);
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

	Engine::Object* ObjectProperty::object(void* context)
	{
		return *address_as<Engine::Object*>(context);
	}

	bool ObjectProperty::object(void* context, Engine::Object* object)
	{
		if (object == nullptr || object->class_instance()->is_a(class_instance()))
		{
			(*address_as<Engine::Object*>(context)) = object;
			return true;
		}
		return false;
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

	bool ArrayProperty::serialize(void* object, Archive& ar)
	{
		size_t elements = length(object);
		ar.serialize(elements);

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

	const String& ArrayProperty::index_name(const void* object, size_t index) const
	{
		static Vector<String> index_names;

		while (index >= index_names.size())
		{
			index_names.push_back(Strings::format("{}", index_names.size() + 1));
		}

		return index_names[index];
	}

	size_t ReflObjectProperty::size() const
	{
		return sizeof(Refl::Object*);
	}

	bool ReflObjectProperty::serialize(void* context, Archive& ar)
	{
		if (ar.is_reading())
		{
			String name;
			ar.serialize_string(name);

			Object* obj = nullptr;

			if (!name.empty())
			{
				obj = static_find(name);

				if (!obj)
				{
					error_log("ReflObjectProperty", "Failed to find reflection object '%s'", name.c_str());
					return false;
				}
			}
			object(context, obj);
			return true;
		}
		else
		{
			String name = "";
			Object* obj = object(context);

			if (obj)
			{
				name = obj->full_name().c_str();
			}
			ar.serialize_string(name);
		}
		return true;
	}

	Refl::Object* ReflObjectProperty::object(void* context)
	{
		return *address_as<Refl::ObjectProperty*>(context);
	}

	bool ReflObjectProperty::object(void* context, Refl::Object* object)
	{
		if (object == nullptr || object->refl_class_info()->is_a(info()))
		{
			(*address_as<Refl::Object*>(context)) = object;
			return true;
		}

		return false;
	}

	const Engine::Refl::Object* ReflObjectProperty::object(const void* context) const
	{
		return *address_as<const Refl::ObjectProperty*>(context);
	}

	Class* SubClassProperty::class_instance(void* context)
	{
		return *address_as<Refl::Class*>(context);
	}

	bool SubClassProperty::class_instance(void* context, Refl::Class* instance)
	{
		return object(context, instance);
	}

	const Class* SubClassProperty::class_instance(const void* context) const
	{
		return *address_as<const Refl::Class*>(context);
	}

	Refl::ClassInfo* SubClassProperty::info() const
	{
		return Class::static_refl_class_info();
	}

	bool FlagsProperty::serialize(void* object, Archive& ar)
	{
		byte* flags = address_as<byte>(object);
		return ar.serialize_memory(flags, size());
	}

	static void on_init()
	{
		{
			using T = Property::Flag;

			ScriptEnumRegistrar r("Engine::Refl::Property::Flag");
			r.set("property", 0);
			r.set("is_read_only", T::IsReadOnly);
			r.set("is_transient", T::IsTransient);
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
