#pragma once
#include <Core/etl/flat_map.hpp>
#include <Core/etl/function.hpp>
#include <Core/etl/variant.hpp>
#include <UI/types.hpp>
#include <imgui.h>

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
		Markup::PropertyPath path;
		Markup::ValueDesc value;
		Markup::SourceLocation location;
	};

	struct StyleSelector {
		Name name;
		StyleState states = StyleState::Undefined;
		u32 specificity   = 0;
	};

	using StyleValue = Variant<f32, Vec2, Vec3, Vec4, ImVec2, ImVec4>;

	struct StyleTransition {
		Name property;
		f32 duration = 0.15f;
		Ease ease    = Ease::OutCubic;
		f32 delay    = 0.0f;
	};

	struct StyleRule {
		StyleSelector selector;
		Vector<StyleProperty> properties;
		Vector<StyleTransition> transitions;
		u32 order = 0;
	};

	using StylePropertyVisitor = FunctionRef<void(const StyleProperty&)>;
	using StyleRuleVisitor     = FunctionRef<void(const StyleRule&)>;
	using StyleRuleSource      = FunctionRef<void(const StyleRuleVisitor&)>;

	struct StyleAnimationTrack {
		StyleValue from;
		StyleValue to;
		f32 elapsed  = 0.0f;
		f32 duration = 0.15f;
		Ease ease    = Ease::OutCubic;
		f32 delay    = 0.0f;
		bool active  = false;
	};

	class StyleInstance
	{
	private:
		FlatMap<Name, StyleValue> m_values;
		FlatMap<Name, StyleValue> m_targets;
		FlatMap<Name, StyleAnimationTrack> m_tracks;
		Vector<StyleProperty> m_properties;
		Vector<StyleTransition> m_transitions;
		u32 m_revision     = 0;
		StyleState m_state = StyleState::Undefined;

	public:
		StyleInstance& update(usize revision, StyleState state, const StyleRuleSource& source, const StylePropertyVisitor& apply);
		StyleInstance& clear();
	};

	class StyleSheet
	{
	private:
		Vector<StyleRule> m_rules;
		FlatMap<Name, Vector<usize>> m_named_rules;
		u32 m_revision = 0;

	public:
		StyleSheet& clear();
		StyleSheet& add_rule(const StyleSelector& selector, const Vector<StyleProperty>& properties,
		                     const Vector<StyleTransition>& transitions = {});
		void resolve(const Element* element, StyleState states, const StyleRuleVisitor& visitor) const;

		inline const Vector<StyleRule>& rules() const { return m_rules; }
		inline u32 revision() const { return m_revision; }
	};
}// namespace Trinex::UI
