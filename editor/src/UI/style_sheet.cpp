#include <UI/element.hpp>
#include <UI/api.hpp>
#include <UI/style_sheet.hpp>
#include <Core/math/math.hpp>

namespace Trinex::UI
{
	static bool value_to_f32(f32& dst, const Markup::ValueDesc& src)
	{
		if (const auto* value = etl::get_if<f32>(&src.value))
		{
			dst = *value;
			return true;
		}

		if (const auto* value = etl::get_if<i32>(&src.value))
		{
			dst = static_cast<f32>(*value);
			return true;
		}

		return false;
	}

	static bool value_to_vec4(Vec4& dst, const Markup::ValueDesc& src)
	{
		const auto* list = etl::get_if<Markup::Container>(&src.value);

		if (list == nullptr || list->size() != 4)
		{
			return false;
		}

		return value_to_f32(dst.x, (*list)[0]) && value_to_f32(dst.y, (*list)[1]) && value_to_f32(dst.z, (*list)[2]) &&
		       value_to_f32(dst.w, (*list)[3]);
	}

	static bool style_value(StyleValue& dst, const Markup::ValueDesc& src)
	{
		Vec4 vec4;

		if (value_to_vec4(vec4, src))
		{
			dst = vec4;
			return true;
		}

		f32 number;

		if (value_to_f32(number, src))
		{
			dst = number;
			return true;
		}

		return false;
	}

	static Markup::ValueDesc value_desc(const StyleValue& value, const Markup::SourceLocation& location)
	{
		Markup::ValueDesc desc;
		desc.location = location;

		if (const auto* number = etl::get_if<f32>(&value))
		{
			desc.value = *number;
			return desc;
		}

		const Vec4& vec4 = etl::get<Vec4>(value);
		Markup::Container list;
		list.push_back({.value = vec4.x, .location = location});
		list.push_back({.value = vec4.y, .location = location});
		list.push_back({.value = vec4.z, .location = location});
		list.push_back({.value = vec4.w, .location = location});
		desc.value = etl::move(list);
		return desc;
	}

	static bool same_value(const StyleValue& lhs, const StyleValue& rhs)
	{
		if (lhs.index() != rhs.index())
		{
			return false;
		}

		if (const auto* lhs_number = etl::get_if<f32>(&lhs))
		{
			return *lhs_number == etl::get<f32>(rhs);
		}

		return etl::get<Vec4>(lhs) == etl::get<Vec4>(rhs);
	}

	static StyleValue lerp_value(const StyleValue& from, const StyleValue& to, f32 t)
	{
		if (from.index() != to.index())
		{
			return to;
		}

		if (const auto* from_number = etl::get_if<f32>(&from))
		{
			return Math::lerp(*from_number, etl::get<f32>(to), t);
		}

		return Math::lerp(etl::get<Vec4>(from), etl::get<Vec4>(to), t);
	}

	static const StyleTransition* find_transition(const Vector<StyleTransition>& transitions, Name property)
	{
		for (const StyleTransition& transition : transitions)
		{
			if (transition.property == property)
			{
				return &transition;
			}
		}

		return nullptr;
	}

	static bool has_later_property(const Vector<StyleProperty>& properties, usize index)
	{
		for (usize next = index + 1; next < properties.size(); ++next)
		{
			if (properties[next].name == properties[index].name)
			{
				return true;
			}
		}

		return false;
	}

	StyleInstance& StyleInstance::clear()
	{
		m_values.clear();
		m_targets.clear();
		m_tracks.clear();
		return *this;
	}

	ComputedStyle StyleInstance::update(const ComputedStyle& style)
	{
		ComputedStyle result;
		result.transitions = style.transitions;

		const f32 dt = Math::max(0.0f, delta_time());

		for (usize index = 0; index < style.properties.size(); ++index)
		{
			const StyleProperty& property = style.properties[index];

			StyleValue target;
			const StyleTransition* transition = find_transition(style.transitions, property.name);

			if (has_later_property(style.properties, index) || transition == nullptr || transition->duration <= 0.0f ||
			    !style_value(target, property.value))
			{
				result.properties.push_back(property);
				continue;
			}

			const bool initialized    = m_targets.contains(property.name);
			StyleValue& stored_target = m_targets[property.name];
			StyleValue& current       = m_values[property.name];

			if (!initialized)
			{
				stored_target = target;
				current       = target;
				result.properties.push_back(property);
				continue;
			}

			if (!same_value(stored_target, target))
			{
				StyleAnimationTrack& track = m_tracks[property.name];
				track.from                 = current;
				track.to                   = target;
				track.elapsed              = 0.0f;
				track.duration             = transition->duration;
				track.ease                 = transition->ease;
				track.delay                = transition->delay;
				track.active               = true;
				stored_target              = target;
			}

			StyleAnimationTrack& track = m_tracks[property.name];

			if (track.active)
			{
				track.elapsed += dt;
				const f32 time = Math::max(0.0f, track.elapsed - track.delay);
				const f32 t    = track.duration <= 0.0f ? 1.0f : Math::clamp(time / track.duration, 0.0f, 1.0f);
				current     = lerp_value(track.from, track.to, apply_ease(t, track.ease));

				if (t >= 1.0f)
				{
					current      = track.to;
					track.active = false;
				}

				StyleProperty animated = property;
				animated.value         = value_desc(current, property.location);
				result.properties.push_back(animated);
			}
			else
			{
				current = stored_target;
				result.properties.push_back(property);
			}
		}

		return result;
	}

	StyleSheet& StyleSheet::clear()
	{
		m_rules.clear();
		m_named_rules.clear();
		return *this;
	}

	StyleSheet& StyleSheet::add_rule(const StyleSelector& selector, const Vector<StyleProperty>& properties,
	                                 const Vector<StyleTransition>& transitions, const Markup::SourceLocation& location)
	{
		const usize index = m_rules.size();

		StyleRule& rule = m_rules.emplace_back();
		rule.selector    = selector;
		rule.properties  = properties;
		rule.transitions = transitions;
		rule.location    = location;
		rule.order       = static_cast<u32>(index);

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

			for (const StyleTransition& transition : rule.transitions)
			{
				result.transitions.push_back(transition);
			}
		}

		return result;
	}
}// namespace Trinex::UI
