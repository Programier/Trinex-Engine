#include <Core/archive.hpp>
#include <Core/object.hpp>
#include <Core/property.hpp>
#include <Core/reflection/struct.hpp>

namespace Engine
{
	ArrayPropertyValue::ArrayPropertyValue(const void* _instance, class Property* _element_type)
	    : instace(_instance), element_type(_element_type)
	{}

	StructPropertyValue::StructPropertyValue(const void* _instance, class Refl::Struct* _struct)
	    : instace(_instance), struct_instance(_struct)
	{}

	PropertyValue::PropertyValue() : Any()
	{
		m_type = PropertyType::Undefined;
	}


#define declare_prop_constructor(type, enum_type)                                                                                \
	PropertyValue::PropertyValue(const type& value) : Any(value)                                                                 \
	{                                                                                                                            \
		m_type = PropertyType::enum_type;                                                                                        \
	}                                                                                                                            \
	PropertyValue& PropertyValue::operator=(const type& value)                                                                   \
	{                                                                                                                            \
		static_cast<Any&>(*this) = value;                                                                                        \
		m_type                   = PropertyType::enum_type;                                                                      \
		return *this;                                                                                                            \
	}

#define check_prop_type(type)                                                                                                    \
	if (m_type != PropertyType::type)                                                                                            \
		return {};

	PropertyValue::PropertyValue(const PropertyValue&)            = default;
	PropertyValue::PropertyValue(PropertyValue&&)                 = default;
	PropertyValue& PropertyValue::operator=(const PropertyValue&) = default;
	PropertyValue& PropertyValue::operator=(PropertyValue&&)      = default;

	declare_prop_constructor(ArrayPropertyValue, Array);
	declare_prop_constructor(StructPropertyValue, Struct);

	PropertyType PropertyValue::type() const
	{
		return m_type;
	}

	EnumerateType PropertyValue::enum_v() const
	{
		check_prop_type(Enum);
		return cast<EnumerateType>();
	}

	Object* PropertyValue::object_v() const
	{
		check_prop_type(Object);
		return cast<Object*>();
	}

	Object* PropertyValue::object_referece_v() const
	{
		check_prop_type(ObjectReference);
		return cast<Object*>();
	}

	StructPropertyValue PropertyValue::struct_v() const
	{
		check_prop_type(Struct);
		return cast<StructPropertyValue>();
	}

	ArrayPropertyValue PropertyValue::array_v() const
	{
		check_prop_type(Array);
		return cast<ArrayPropertyValue>();
	}

	PropertyValue::operator bool() const
	{
		return has_value();
	}

	Property::Property(const Name& name, const String& description, const Name& group, BitMask flags)
	    : m_name(name), m_group(group), m_description(description), m_flags(flags)
	{}

	Property& Property::name(const Name& new_name)
	{
		m_name = new_name;
		return *this;
	}

	const Name& Property::name() const
	{
		return m_name;
	}

	const Name& Property::group() const
	{
		return m_group;
	}

	const String& Property::description() const
	{
		return m_description;
	}

	const Flags<Property::Flag>& Property::flags() const
	{
		return m_flags;
	}

	size_t Property::type_id() const
	{
		return 0;
	}

	Refl::Struct* Property::struct_instance()
	{
		return nullptr;
	}

	class Refl::Enum* Property::enum_instance()
	{
		return nullptr;
	}

	bool Property::is_const() const
	{
		return m_flags.has_any(Flags<Flag>(IsConst) | Flags<Flag>(IsNativeConst));
	}

	bool Property::is_private() const
	{
		return m_flags(IsPrivate);
	}

	bool Property::is_serializable() const
	{
		return !m_flags(IsNativeConst) && !m_flags(IsNotSerializable);
	}

	bool Property::is_hidden() const
	{
		return m_flags(IsHidden);
	}

	bool Property::is_color() const
	{
		return m_flags(IsColor);
	}

	static FORCE_INLINE List<Property*> collect_serializable_properties(Refl::Struct* self)
	{
		List<Property*> result;

		for (auto& prop : self->properties())
		{
			if (prop->is_serializable())
			{
				result.push_back(prop);
			}
		}
		return result;
	}

	static bool serialize_object_properies(Refl::Struct* self, void* object, Archive& ar)
	{
		if (ar.is_saving())
		{
			auto properties = collect_serializable_properties(self);

			size_t count = properties.size();
			ar & count;

			Vector<size_t> offsets(count + 1, 0);
			auto start_pos = ar.position();
			ar.write_data(reinterpret_cast<const byte*>(offsets.data()), offsets.size() * sizeof(size_t));

			count = 0;
			for (auto& prop : properties)
			{
				Name name      = prop->name();
				offsets[count] = ar.position() - start_pos;
				ar & name;
				prop->archive_process(object, ar);
				++count;
			}

			auto end_pos   = ar.position();
			offsets[count] = end_pos - start_pos;

			ar.position(start_pos);
			ar.write_data(reinterpret_cast<const byte*>(offsets.data()), offsets.size() * sizeof(size_t));
			ar.position(end_pos);
		}
		else if (ar.is_reading())
		{
			size_t count = 0;
			ar & count;

			Vector<size_t> offsets(count + 1, 0);
			auto start_pos = ar.position();
			ar.read_data(reinterpret_cast<byte*>(offsets.data()), offsets.size() * sizeof(size_t));

			Name name;

			for (size_t i = 0; i < count; ++i)
			{
				ar.position(start_pos + offsets[i]);

				ar & name;
				Property* prop = self->find_property(name);

				if (prop && prop->is_serializable())
				{
					prop->archive_process(object, ar);
				}
			}

			ar.position(start_pos + offsets.back());
		}

		self = self->parent();

		if (self)
		{
			return serialize_object_properies(self, object, ar);
		}

		return ar;
	}

	bool Property::serialize_properies(class Refl::Struct* self, void* object, Archive& ar)
	{
		return serialize_object_properies(self, object, ar);
	}

	bool Property::archive_process(void* object, Archive& ar)
	{
		if (!is_serializable())
			return false;

		PropertyType prop_type = type();
		if (static_cast<EnumerateType>(prop_type) <= static_cast<EnumerateType>(PropertyType::LastPrimitive))
		{
			if (ar.is_reading())
			{
				ar.read_data(reinterpret_cast<byte*>(prop_address(object)), size());
			}
			else if (ar.is_saving())
			{
				ar.write_data(reinterpret_cast<const byte*>(prop_address(object)), size());
			}
		}
		else if (prop_type == PropertyType::Object)
		{
			Object* new_object = property_value(object).object_v();
			trinex_always_check(new_object != nullptr, "Property with type 'Object' can't be nullptr!");
			return new_object->archive_process(ar);
		}
		else if (prop_type == PropertyType::ObjectReference)
		{
			Object*& new_object = *reinterpret_cast<Object**>(prop_address(object));
			return ar.serialize_reference(new_object);
		}
		return ar;
	}

	Property::~Property()
	{}


	bool ArrayPropertyInterface::archive_process(void* object, Archive& ar)
	{
		if (!is_serializable())
			return false;

		Property* e_type = element_type();
		if (e_type == nullptr)
			return false;

		size_t count = elements_count(object);
		ar & count;

		if (ar.is_reading())
		{
			resize(object, count);
		}

		for (size_t i = 0; i < count; ++i)
		{
			void* element = at(object, i);
			e_type->archive_process(element, ar);
		}

		return ar;
	}

	bool Object::serialize_object_properties(Archive& ar)
	{
		Refl::Struct* self = reinterpret_cast<Refl::Struct*>(class_instance());
		if (self == nullptr)
			return false;

		return serialize_object_properies(self, this, ar);
	}

	ENGINE_EXPORT Name default_array_element_name(ArrayPropertyInterface* interface, void* object, size_t index)
	{
		static thread_local Vector<String> elements;

		if (index >= interface->elements_count(object))
		{
			return Name::out_of_range;
		}

		while (index >= elements.size())
		{
			elements.push_back(Strings::format("{}", elements.size()));
		}

		return elements[index].c_str();
	}

	ENGINE_EXPORT Name default_array_object_element_name(class ArrayPropertyInterface* interface, void* object, size_t index)
	{
		if (index >= interface->elements_count(object))
		{
			return Name::out_of_range;
		}

		auto element_prop = interface->element_type();

		if (element_prop && is_in<PropertyType::Object, PropertyType::ObjectReference>(element_prop->type()))
		{
			if (void* element = reinterpret_cast<Object*>(interface->at(object, index)))
			{
				Object* element_object = interface->element_type()->property_value(element).cast<Object*>();
				return element_object->name();
			}
		}

		return default_array_element_name(interface, object, index);
	}

	ArrayPropertyInterface::ArrayPropertyInterface(const Name& name, const String& description, const Name& group, BitMask flags)
	    : Property(name, description, group, flags)
	{
		m_element_name_callback = default_array_element_name;
	}

	ArrayPropertyElementNameCallback ArrayPropertyInterface::element_name_callback() const
	{
		return m_element_name_callback;
	}

	ArrayPropertyInterface& ArrayPropertyInterface::element_name_callback(ArrayPropertyElementNameCallback callback)
	{
		if (callback)
		{
			m_element_name_callback = callback;
		}
		return *this;
	}

	Name ArrayPropertyInterface::element_name(void* object, size_t index)
	{
		if (index >= elements_count(object))
		{
			return Name::out_of_range;
		}

		return m_element_name_callback(this, object, index);
	}
}// namespace Engine
