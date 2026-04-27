#include <Core/archive.hpp>
#include <Core/etl/templates.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_type_info.hpp>

namespace Trinex::Refl
{
	trinex_implement_reflect_type(Trinex::Refl::Property)
	{
		using T = Property::Flag;

		ScriptEnumRegistrar e("Trinex::Refl::Property::Flag");
		e.set("property", 0);
		e.set("is_read_only", T::IsReadOnly);
		e.set("is_transient", T::IsTransient);
		e.set("is_hidden", T::IsHidden);
		e.set("inline_single_field", T::InlineSingleField);
		e.set("inline", T::Inline);
	}

	trinex_implement_reflect_type(Trinex::Refl::PrimitiveProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::BooleanProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::IntegerProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::FloatProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::AngleProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::EnumProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::ColorProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::LinearColorProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::VectorProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::MatrixProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::QuaternionProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::StringProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::NameProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::PathProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::ObjectProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::StructProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::ArrayProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::ReflObjectProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::SubClassProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::FlagsProperty) {}
	trinex_implement_reflect_type(Trinex::Refl::VirtualProperty) {}

	void Property::trigger_object_event(const PropertyChangedEvent& event)
	{
		Trinex::Object* object = reinterpret_cast<Trinex::Object*>(event.context);
		object->on_property_changed(event);
	}

	Property::Property(BitMask flags) : m_flags(flags) {}

	const String& Property::property_name(const void* context)
	{
		return display_name();
	}

	Property& Property::on_property_changed(const PropertyChangedEvent& event)
	{
		return *this;
	}

	bool Property::render(PropertyRenderer& renderer, void* context)
	{
		if (m_render_function)
		{
			return m_render_function(renderer, *this, context);
		}

		return renderer.render_default(*this, context);
	}

	Property& Property::item_flags(BitMask flags)
	{
		if (auto* array = Refl::Object::instance_cast<ArrayProperty>(this))
		{
			return array->element_flags(flags);
		}

		if (auto* vector = Refl::Object::instance_cast<VectorProperty>(this))
		{
			return vector->element_flags(flags);
		}

		if (auto* matrix = Refl::Object::instance_cast<MatrixProperty>(this))
		{
			return matrix->row_flags(flags);
		}

		return *this;
	}

	Property& Property::render_function(RenderFunction function)
	{
		m_render_function = std::move(function);
		return *this;
	}

	void Property::register_layout(ScriptClassRegistrar& r, ClassInfo* info, DownCast downcast)
	{
		Super::register_layout(r, info, downcast);
	}

	Property::~Property() {}

	bool PrimitiveProperty::serialize(void* object, Archive& ar)
	{
		u8* prop = reinterpret_cast<u8*>(address(object));
		ar.serialize_memory(prop, size());
		return ar;
	}

	VectorProperty& VectorProperty::element_flags(BitMask flags)
	{
		m_element_property_flags = flags;
		return *this;
	}

	MatrixProperty& MatrixProperty::row_flags(BitMask flags)
	{
		m_row_property_flags = flags;
		return *this;
	}

	usize BooleanProperty::size() const
	{
		return sizeof(bool);
	}

	usize BooleanProperty::alignment() const
	{
		return alignof(bool);
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

	usize StringProperty::size() const
	{
		return sizeof(String);
	}

	usize StringProperty::alignment() const
	{
		return alignof(String);
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

	usize ObjectProperty::size() const
	{
		return sizeof(Trinex::Object*);
	}

	usize ObjectProperty::alignment() const
	{
		return alignof(Trinex::Object*);
	}

	bool ObjectProperty::serialize(void* object, Archive& ar)
	{
		Trinex::Object*& instance = *address_as<Trinex::Object*>(object);

		if (m_is_composite)
		{
			return instance->serialize(ar);
		}
		else
		{
			return ar.serialize_object_ref(instance);
		}
	}

	Trinex::Object* ObjectProperty::object(void* context)
	{
		return *address_as<Trinex::Object*>(context);
	}

	bool ObjectProperty::object(void* context, Trinex::Object* object)
	{
		if (object == nullptr || object->class_instance()->is_a(class_instance()))
		{
			(*address_as<Trinex::Object*>(context)) = object;
			return true;
		}
		return false;
	}

	const Trinex::Object* ObjectProperty::object(const void* context) const
	{
		return *address_as<Trinex::Object*>(context);
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
		usize elements = length(object);
		ar.serialize(elements);

		if (ar.is_reading())
		{
			resize(object, elements);
		}

		auto element_prop = element_property();

		for (usize i = 0; i < elements; ++i)
		{
			void* element = at(object, i);
			element_prop->serialize(element, ar);
		}

		return ar;
	}

	ArrayProperty& ArrayProperty::element_flags(BitMask flags)
	{
		m_element_property_flags = flags;
		return *this;
	}

	const String& ArrayProperty::index_name(const void* object, usize index) const
	{
		static Vector<String> index_names;

		while (index >= index_names.size())
		{
			index_names.push_back(Strings::format("{}", index_names.size() + 1));
		}

		return index_names[index];
	}

	usize ReflObjectProperty::size() const
	{
		return sizeof(Refl::Object*);
	}

	usize ReflObjectProperty::alignment() const
	{
		return alignof(Refl::Object*);
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

	const Trinex::Refl::Object* ReflObjectProperty::object(const void* context) const
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
		u8* flags = address_as<u8>(object);
		return ar.serialize_memory(flags, size());
	}

	void* VirtualProperty::address(void* context)
	{
		trinex_unreachable_msg("Cannot use Property::address method on virtual properties!");
		return nullptr;
	}

	const void* VirtualProperty::address(const void* context) const
	{
		trinex_unreachable_msg("Cannot use Property::address method on virtual properties!");
		return nullptr;
	}

	bool VirtualProperty::serialize(void* object, Archive& ar)
	{
		if (ar.is_reading())
		{
			Any value  = construct_value();
			void* prop = value.address();

			if (property()->serialize(prop, ar))
			{
				setter(object, value);
				return true;
			}

			return false;
		}
		else if (ar.is_saving())
		{
			Any value = getter(object);
			return property()->serialize(value.address(), ar);
		}

		return false;
	}
}// namespace Trinex::Refl
