#include <Core/etl/templates.hpp>
#include <Core/string_functions.hpp>
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
				return prop.setter(object, prop.type, value);
			}

			self = self->m_parent;
		}

		return false;
	}

	bool Type::assign(void* dst, Type* type, const Markup::ValueDesc& value)
	{
		return false;
	}

	bool Type::assign(bool* dst, Type* type, const Markup::ValueDesc& value)
	{
		if (const bool* source = etl::get_if<bool>(&value.value))
		{
			*dst = *source;
			return true;
		}

		return false;
	}

	bool Type::assign(i32* dst, Type* type, const Markup::ValueDesc& value)
	{
		if (const i32* source = etl::get_if<i32>(&value.value))
		{
			*dst = *source;
			return true;
		}

		return false;
	}

	bool Type::assign(f32* dst, Type* type, const Markup::ValueDesc& value)
	{
		if (const f32* source = etl::get_if<f32>(&value.value))
		{
			*dst = *source;
			return true;
		}

		if (const i32* source = etl::get_if<i32>(&value.value))
		{
			*dst = static_cast<f32>(*source);
			return true;
		}

		return false;
	}

	bool Type::assign(String* dst, Type* type, const Markup::ValueDesc& value)
	{
		if (const String* source = etl::get_if<String>(&value.value))
		{
			*dst = *source;
			return true;
		}

		if (const Markup::Identifier* source = etl::get_if<Markup::Identifier>(&value.value))
		{
			*dst = *source;
			return true;
		}

		if (const Markup::LocalizationKey* source = etl::get_if<Markup::LocalizationKey>(&value.value))
		{
			*dst = *source;
			return true;
		}

		if (const Markup::BindingPath* source = etl::get_if<Markup::BindingPath>(&value.value))
		{
			*dst = *source;
			return true;
		}

		return false;
	}

	bool Type::assign(Markup::LocalizationKey* dst, Type* type, const Markup::ValueDesc& value)
	{
		if (const Markup::LocalizationKey* source = etl::get_if<Markup::LocalizationKey>(&value.value))
		{
			*dst = *source;
			return true;
		}

		return false;
	}

	bool Type::assign(Markup::BindingPath* dst, Type* type, const Markup::ValueDesc& value)
	{
		if (const Markup::BindingPath* source = etl::get_if<Markup::BindingPath>(&value.value))
		{
			*dst = *source;
			return true;
		}

		return false;
	}

	bool Type::assign(Markup::Identifier* dst, Type* type, const Markup::ValueDesc& value)
	{
		if (const Markup::Identifier* source = etl::get_if<Markup::Identifier>(&value.value))
		{
			*dst = *source;
			return true;
		}

		return false;
	}

	ElementRegistry* ElementRegistry::instance()
	{
		static ElementRegistry registry;
		return &registry;
	}
}// namespace Trinex::UI
