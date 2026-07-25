#include <UI/element.hpp>
#include <UI/style_sheet.hpp>

namespace Trinex::UI
{
	StyleSheet& StyleSheet::clear()
	{
		m_rules.clear();
		m_named_rules.clear();
		return *this;
	}

	StyleSheet& StyleSheet::add_rule(const StyleSelector& selector, const Vector<StyleProperty>& properties,
	                                 const Markup::SourceLocation& location)
	{
		const usize index = m_rules.size();

		StyleRule& rule = m_rules.emplace_back();
		rule.selector   = selector;
		rule.properties = properties;
		rule.location   = location;
		rule.order      = static_cast<u32>(index);

		m_named_rules[selector.name].push_back(index);
		return *this;
	}

	static bool style_matches(const Element* element, const StyleSelector& selector, StyleState states)
	{
		if (element == nullptr || !selector.name.is_valid())
		{
			return false;
		}

		if ((selector.states & states) != selector.states)
		{
			return false;
		}

		if (element->type_name() == selector.name)
		{
			return true;
		}

		for (const Name& style : element->styles())
		{
			if (style == selector.name)
			{
				return true;
			}
		}

		return false;
	}

	ComputedStyle StyleSheet::resolve(const Element* element, StyleState states) const
	{
		ComputedStyle result;

		for (const StyleRule& rule : m_rules)
		{
			if (!style_matches(element, rule.selector, states))
			{
				continue;
			}

			for (const StyleProperty& property : rule.properties)
			{
				result.properties.push_back(property);
			}
		}

		return result;
	}
}// namespace Trinex::UI
