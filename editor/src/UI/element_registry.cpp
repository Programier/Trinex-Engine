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
			auto it = m_properties.find(name);

			if (it != m_properties.end())
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
		return false;
	}

	bool Type::assign(i32* dst, Type* type, const Markup::ValueDesc& value)
	{
		return false;
	}

	bool Type::assign(f32* dst, Type* type, const Markup::ValueDesc& value)
	{
		return false;
	}

	bool Type::assign(String* dst, Type* type, const Markup::ValueDesc& value)
	{
		auto visitor = overloaded{
		        [](Markup::Null value) -> String { return "false"; },
		        [](bool value) -> String { return value ? "true" : "false"; },
		        [](i32 value) -> String { return Strings::format("{}", value); },
		        [](f32 value) -> String { return Strings::format("{}", value); },
		        [](const String& value) -> String { return value; },
		        [](const Markup::LocalizationKey& value) -> String { return value; },
		        [](const Markup::BindingPath& value) -> String { return value; },
		        [](const Markup::Identifier& value) -> String { return value; },
		        [](const Markup::Container& value) -> String { return ""; },
		};

		(*dst) = etl::visit(visitor, value.value);
		return true;
	}

	bool Type::assign(Markup::LocalizationKey* dst, Type* type, const Markup::ValueDesc& value)
	{
		return false;
	}

	bool Type::assign(Markup::BindingPath* dst, Type* type, const Markup::ValueDesc& value)
	{
		return false;
	}

	bool Type::assign(Markup::Identifier* dst, Type* type, const Markup::ValueDesc& value)
	{
		return false;
	}

	ElementRegistry* ElementRegistry::instance()
	{
		static ElementRegistry registry;
		return &registry;
	}
}// namespace Trinex::UI
