#include <Core/math/math.hpp>
#include <UI/element.hpp>
#include <UI/reflection.hpp>
#include <UI/style_sheet.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	static f32 apply_style_ease(f32 t, Ease mode)
	{
		t = Math::clamp(t, 0.0f, 1.0f);

		switch (mode)
		{
			case Ease::Linear: return t;
			case Ease::InQuad: return t * t;
			case Ease::OutQuad: return 1.0f - (1.0f - t) * (1.0f - t);
			case Ease::InOutQuad: return t < 0.5f ? 2.0f * t * t : 1.0f - Math::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f;
			case Ease::InCubic: return t * t * t;
			case Ease::OutCubic: return 1.0f - Math::pow(1.0f - t, 3.0f);
			case Ease::InOutCubic: return t < 0.5f ? 4.0f * t * t * t : 1.0f - Math::pow(-2.0f * t + 2.0f, 3.0f) * 0.5f;
			case Ease::InExpo: return t == 0.0f ? 0.0f : Math::pow(2.0f, 10.0f * t - 10.0f);
			case Ease::OutExpo: return t == 1.0f ? 1.0f : 1.0f - Math::pow(2.0f, -10.0f * t);
			case Ease::InOutExpo:
				if (t == 0.0f || t == 1.0f)
					return t;
				return t < 0.5f ? Math::pow(2.0f, 20.0f * t - 10.0f) * 0.5f : (2.0f - Math::pow(2.0f, -20.0f * t + 10.0f)) * 0.5f;
			case Ease::OutBack:
			{
				const f32 c1 = 1.70158f;
				const f32 c3 = c1 + 1.0f;
				return 1.0f + c3 * Math::pow(t - 1.0f, 3.0f) + c1 * Math::pow(t - 1.0f, 2.0f);
			}
		}

		return t;
	}

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

	static bool value_to_vec3(Vec3& dst, const Markup::ValueDesc& src)
	{
		const auto* list = etl::get_if<Markup::Container>(&src.value);

		if (list == nullptr || list->size() != 3)
		{
			return false;
		}

		return value_to_f32(dst.x, (*list)[0]) && value_to_f32(dst.y, (*list)[1]) && value_to_f32(dst.z, (*list)[2]);
	}

	static bool value_to_vec2(Vec2& dst, const Markup::ValueDesc& src)
	{
		const auto* list = etl::get_if<Markup::Container>(&src.value);

		if (list == nullptr || list->size() != 2)
		{
			return false;
		}

		return value_to_f32(dst.x, (*list)[0]) && value_to_f32(dst.y, (*list)[1]);
	}

	static bool style_value(StyleValue& dst, const Markup::ValueDesc& src)
	{
		Vec4 vec4;

		if (value_to_vec4(vec4, src))
		{
			dst = vec4;
			return true;
		}

		Vec3 vec3;

		if (value_to_vec3(vec3, src))
		{
			dst = vec3;
			return true;
		}

		Vec2 vec2;

		if (value_to_vec2(vec2, src))
		{
			dst = vec2;
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

	static Markup::ValueDesc value_desc(const Vec2& value, const Markup::SourceLocation& location)
	{
		Markup::ValueDesc desc;
		desc.location = location;

		Markup::Container list;
		list.push_back({.value = value.x, .location = location});
		list.push_back({.value = value.y, .location = location});
		desc.value = etl::move(list);
		return desc;
	}

	static Markup::ValueDesc value_desc(const Vec3& value, const Markup::SourceLocation& location)
	{
		Markup::ValueDesc desc;
		desc.location = location;

		Markup::Container list;
		list.push_back({.value = value.x, .location = location});
		list.push_back({.value = value.y, .location = location});
		list.push_back({.value = value.z, .location = location});
		desc.value = etl::move(list);
		return desc;
	}

	static Markup::ValueDesc value_desc(const Vec4& value, const Markup::SourceLocation& location)
	{
		Markup::ValueDesc desc;
		desc.location = location;

		Markup::Container list;
		list.push_back({.value = value.x, .location = location});
		list.push_back({.value = value.y, .location = location});
		list.push_back({.value = value.z, .location = location});
		list.push_back({.value = value.w, .location = location});
		desc.value = etl::move(list);
		return desc;
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

		if (const auto* vec2 = etl::get_if<Vec2>(&value))
		{
			return value_desc(*vec2, location);
		}

		if (const auto* vec3 = etl::get_if<Vec3>(&value))
		{
			return value_desc(*vec3, location);
		}

		if (const auto* vec4 = etl::get_if<Vec4>(&value))
		{
			return value_desc(*vec4, location);
		}

		if (const auto* vec2 = etl::get_if<ImVec2>(&value))
		{
			return value_desc(Vec2(vec2->x, vec2->y), location);
		}

		const ImVec4& vec4 = etl::get<ImVec4>(value);
		return value_desc(Vec4(vec4.x, vec4.y, vec4.z, vec4.w), location);
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

		if (const auto* lhs_vec2 = etl::get_if<Vec2>(&lhs))
		{
			return *lhs_vec2 == etl::get<Vec2>(rhs);
		}

		if (const auto* lhs_vec3 = etl::get_if<Vec3>(&lhs))
		{
			return *lhs_vec3 == etl::get<Vec3>(rhs);
		}

		if (const auto* lhs_vec4 = etl::get_if<Vec4>(&lhs))
		{
			return *lhs_vec4 == etl::get<Vec4>(rhs);
		}

		if (const auto* lhs_vec2 = etl::get_if<ImVec2>(&lhs))
		{
			const ImVec2& rhs_vec2 = etl::get<ImVec2>(rhs);
			return lhs_vec2->x == rhs_vec2.x && lhs_vec2->y == rhs_vec2.y;
		}

		const ImVec4& lhs_vec4 = etl::get<ImVec4>(lhs);
		const ImVec4& rhs_vec4 = etl::get<ImVec4>(rhs);
		return lhs_vec4.x == rhs_vec4.x && lhs_vec4.y == rhs_vec4.y && lhs_vec4.z == rhs_vec4.z && lhs_vec4.w == rhs_vec4.w;
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

		if (const auto* from_vec2 = etl::get_if<Vec2>(&from))
		{
			return Math::lerp(*from_vec2, etl::get<Vec2>(to), t);
		}

		if (const auto* from_vec3 = etl::get_if<Vec3>(&from))
		{
			return Math::lerp(*from_vec3, etl::get<Vec3>(to), t);
		}

		if (const auto* from_vec4 = etl::get_if<Vec4>(&from))
		{
			return Math::lerp(*from_vec4, etl::get<Vec4>(to), t);
		}

		if (const auto* from_vec2 = etl::get_if<ImVec2>(&from))
		{
			const ImVec2& to_vec2 = etl::get<ImVec2>(to);
			return ImVec2(Math::lerp(from_vec2->x, to_vec2.x, t), Math::lerp(from_vec2->y, to_vec2.y, t));
		}

		const ImVec4& from_vec4 = etl::get<ImVec4>(from);
		const ImVec4& to_vec4   = etl::get<ImVec4>(to);
		return ImVec4(Math::lerp(from_vec4.x, to_vec4.x, t), Math::lerp(from_vec4.y, to_vec4.y, t),
		              Math::lerp(from_vec4.z, to_vec4.z, t), Math::lerp(from_vec4.w, to_vec4.w, t));
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

		const f32 dt = Math::max(0.0f, ImGui::GetIO().DeltaTime);

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
				current        = lerp_value(track.from, track.to, apply_style_ease(t, track.ease));

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

		StyleRule& rule  = m_rules.emplace_back();
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

		for (auto type = element->type(); type; type = type->parent())
		{
			if (type->name() == selector.name)
			{
				return true;
			}
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
