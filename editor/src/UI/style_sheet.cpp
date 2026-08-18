#include <Core/etl/algorithm.hpp>
#include <Core/etl/allocator.hpp>
#include <Core/etl/vector.hpp>
#include <Core/math/math.hpp>
#include <UI/element.hpp>
#include <UI/reflection.hpp>
#include <UI/style_sheet.hpp>
#include <imgui.h>

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

	template<typename AllocatorType>
	static const StyleTransition* find_transition(const Vector<StyleTransition, AllocatorType>& transitions, Name property)
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

	template<typename AllocatorType>
	static bool same_path(const Vector<Name, AllocatorType>& lhs, const Vector<Name, AllocatorType>& rhs)
	{
		if (lhs.size() != rhs.size())
		{
			return false;
		}

		for (usize index = 0; index < lhs.size(); ++index)
		{
			if (lhs[index] != rhs[index])
			{
				return false;
			}
		}

		return true;
	}

	template<typename AllocatorType>
	static bool has_later_property(const Vector<StyleProperty, AllocatorType>& properties, usize index)
	{
		for (usize next = index + 1; next < properties.size(); ++next)
		{
			if (same_path(properties[next].path, properties[index].path))
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
		m_properties.clear();
		m_transitions.clear();
		m_revision = 0;
		m_state    = StyleState::Undefined;
		return *this;
	}

	StyleInstance& StyleInstance::update(usize revision, StyleState state, const StyleRuleSource& source,
	                                     const StylePropertyVisitor& apply)
	{
		const bool rebuild = m_revision != revision || m_state != state;

		if (rebuild)
		{
			m_properties.clear();
			m_transitions.clear();

			source([this](const StyleRule& rule) {
				for (const StyleProperty& property : rule.properties)
				{
					m_properties.push_back(property);
				}

				for (const StyleTransition& transition : rule.transitions)
				{
					m_transitions.push_back(transition);
				}
			});

			m_revision = revision;
			m_state    = state;
		}

		StackByteAllocator::Mark mark;
		const f32 dt = Math::max(0.0f, ImGui::GetIO().DeltaTime);

		for (usize index = 0; index < m_properties.size(); ++index)
		{
			const StyleProperty& property = m_properties[index];
			const Name property_name      = property.path.empty() ? Name::undefined : property.path.front();

			StyleValue target;
			const StyleTransition* transition = find_transition(m_transitions, property_name);

			if (has_later_property(m_properties, index) || transition == nullptr || transition->duration <= 0.0f ||
			    !style_value(target, property.value))
			{
				apply(property);
				continue;
			}

			const bool initialized    = m_targets.contains(property_name);
			StyleValue& stored_target = m_targets[property_name];
			StyleValue& current       = m_values[property_name];

			if (!initialized)
			{
				stored_target = target;
				current       = target;
				apply(property);
				continue;
			}

			if (!same_value(stored_target, target))
			{
				StyleAnimationTrack& track = m_tracks[property_name];
				track.from                 = current;
				track.to                   = target;
				track.elapsed              = 0.0f;
				track.duration             = transition->duration;
				track.ease                 = transition->ease;
				track.delay                = transition->delay;
				track.active               = true;
				stored_target              = target;
			}

			StyleAnimationTrack& track = m_tracks[property_name];

			if (track.active)
			{
				track.elapsed += dt;
				const f32 time = Math::max(0.0f, track.elapsed - track.delay);
				const f32 t    = track.duration <= 0.0f ? 1.0f : Math::clamp(time / track.duration, 0.0f, 1.0f);
				current        = lerp_value(track.from, track.to, Element::ease(t, track.ease));

				if (t >= 1.0f)
				{
					current      = track.to;
					track.active = false;
				}

				StyleProperty animated = property;
				animated.value         = value_desc(current, property.location);
				apply(animated);
			}
			else
			{
				current = stored_target;
				apply(property);
			}
		}

		return *this;
	}

	StyleSheet& StyleSheet::clear()
	{
		m_rules.clear();
		m_named_rules.clear();
		++m_revision;
		return *this;
	}

	StyleSheet& StyleSheet::add_rule(const StyleSelector& selector, const Vector<StyleProperty>& properties,
	                                 const Vector<StyleTransition>& transitions)
	{
		const usize index = m_rules.size();

		StyleRule& rule  = m_rules.emplace_back();
		rule.selector    = selector;
		rule.properties  = properties;
		rule.transitions = transitions;
		rule.order       = static_cast<u32>(index);

		m_named_rules[selector.name].push_back(index);
		++m_revision;
		return *this;
	}

	static bool state_matches(const StyleSelector& selector, StyleState states)
	{
		return (selector.states & states) == selector.states;
	}

	void StyleSheet::resolve(const Element* element, StyleState states, const StyleRuleVisitor& visitor) const
	{
		StackVector<usize> candidates;

		if (element == nullptr)
		{
			return;
		}

		auto append_rules = [this, &candidates](Name name) {
			if (!name.is_valid())
			{
				return;
			}

			auto it = m_named_rules.find(name);

			if (it == m_named_rules.end())
			{
				return;
			}

			for (usize index : it->second)
			{
				candidates.push_back(index);
			}
		};

		append_rules(element->type()->name());

		for (auto type = element->type(); type; type = type->parent())
		{
			append_rules(type->name());
		}

		for (const Name& style : element->styles())
		{
			append_rules(style);
		}

		etl::sort(candidates.begin(), candidates.end(),
		          [this](usize first, usize second) { return m_rules[first].order < m_rules[second].order; });

		usize prev_index = static_cast<usize>(-1);

		for (usize index : candidates)
		{
			if (index == prev_index)
			{
				continue;
			}

			prev_index            = index;
			const StyleRule& rule = m_rules[index];

			if (!state_matches(rule.selector, states))
			{
				continue;
			}

			visitor(rule);
		}
	}
}// namespace Trinex::UI
