#include <Core/etl/templates.hpp>
#include <Core/string_functions.hpp>
#include <UI/element.hpp>
#include <UI/element_registry.hpp>

namespace Trinex::UI
{
	Type& Type::property(Name name, Type* type, Property::Setter setter)
	{
		trinex_assert(m_properties.find(name) == m_properties.end());
		m_properties.insert({name, Property(type, setter)});
		return *this;
	}

	bool Type::property(void* object, Name name, const Markup::ValueDesc& value)
	{
		Type* self = this;

		while (self)
		{
			auto it = self->m_properties.find(name);

			if (it != self->m_properties.end())
			{
				const Property& prop = it->second;

				FieldBase self;
				self.owner   = nullptr;
				self.address = object;
				self.type    = this;
				return prop.setter(self, prop, value);
			}

			self = self->m_parent;
		}

		return false;
	}

	bool Type::assign(const FieldBase& field, const Markup::ValueDesc& value)
	{
		return false;
	}

	bool Type::assign(const Field<bool>& field, const Markup::ValueDesc& value)
	{
		if (const bool* source = etl::get_if<bool>(&value.value))
		{
			field.ref() = *source;
			return true;
		}

		return false;
	}

	bool Type::assign(const Field<i32>& field, const Markup::ValueDesc& value)
	{
		if (const i32* source = etl::get_if<i32>(&value.value))
		{
			field.ref() = *source;
			return true;
		}

		return false;
	}

	bool Type::assign(const Field<f32>& field, const Markup::ValueDesc& value)
	{
		if (const f32* source = etl::get_if<f32>(&value.value))
		{
			field.ref() = *source;
			return true;
		}

		if (const i32* source = etl::get_if<i32>(&value.value))
		{
			field.ref() = static_cast<f32>(*source);
			return true;
		}

		return false;
	}

	bool Type::assign(const Field<String>& field, const Markup::ValueDesc& value)
	{
		if (const String* source = etl::get_if<String>(&value.value))
		{
			field.ref() = *source;
			return true;
		}

		if (const Markup::Identifier* source = etl::get_if<Markup::Identifier>(&value.value))
		{
			field.ref() = *source;
			return true;
		}

		// if (const Markup::LocalizationKey* source = etl::get_if<Markup::LocalizationKey>(&value.value))
		// {
		// 	*dst = *source;
		// 	return true;
		// }

		if (field.owner)
		{
			void* owner      = field.owner->address;
			Type* owner_type = field.owner->type;

			if (const Markup::BindingPath* source = etl::get_if<Markup::BindingPath>(&value.value))
			{
				if (auto element = Element::cast(owner, owner_type))
				{
					element->bind(field.address, field.type, *source);
				}
				return true;
			}
		}

		return false;
	}

	ElementRegistry* ElementRegistry::instance()
	{
		static ElementRegistry registry;
		return &registry;
	}
}// namespace Trinex::UI
