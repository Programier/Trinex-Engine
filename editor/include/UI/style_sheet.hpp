#pragma once
#include <Core/etl/flat_map.hpp>
#include <UI/types.hpp>

namespace Trinex::UI
{
	class Element;

	struct StyleState {
		enum Enum : u8
		{
			Undefined = 0,
			Hover     = 1 << 0,
			Active    = 1 << 1,
			Focus     = 1 << 2,
			Disabled  = 1 << 3,
		};

		trinex_bitfield_enum_struct(StyleState, u8);
	};

	struct StyleProperty {
		Name name;
		Markup::ValueDesc value;
		Markup::SourceLocation location;
	};

	struct StyleSelector {
		Name name;
		StyleState states = StyleState::Undefined;
		u32 specificity   = 0;
	};

	struct StyleRule {
		StyleSelector selector;
		Vector<StyleProperty> properties;
		Markup::SourceLocation location;
		u32 order = 0;
	};

	struct ComputedStyle {
		Vector<StyleProperty> properties;
	};

	class StyleSheet
	{
	private:
		Vector<StyleRule> m_rules;
		FlatMap<Name, Vector<usize>> m_named_rules;

	public:
		StyleSheet& clear();
		StyleSheet& add_rule(const StyleSelector& selector, const Vector<StyleProperty>& properties,
		                     const Markup::SourceLocation& location = {});
		ComputedStyle resolve(const Element* element, StyleState states = StyleState::Undefined) const;

		inline const Vector<StyleRule>& rules() const { return m_rules; }
	};
}// namespace Trinex::UI
