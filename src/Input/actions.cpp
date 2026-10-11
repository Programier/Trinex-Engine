#include <Input/actions.hpp>
#include <Input/device.hpp>

namespace Trinex
{
	namespace
	{
		f32 value_length_squared(const InputValue& value)
		{
			return value.axes.x * value.axes.x + value.axes.y * value.axes.y + value.axes.z * value.axes.z;
		}

		f32 clamp(f32 value, f32 min, f32 max)
		{
			return value < min ? min : (value > max ? max : value);
		}

		f32 component(const Vector3f& value, u8 index)
		{
			switch (index)
			{
				case 0: return value.x;
				case 1: return value.y;
				case 2: return value.z;
				default: return 0.f;
			}
		}

		bool is_active(const InputEvent& event)
		{
			return event.type == EventType::Press || event.type == EventType::Repeat ||
			       (event.value.type == InputValueType::Boolean && event.value.is_active());
		}

		InputActionPhase combine_phase(InputActionPhase current, InputActionPhase next)
		{
			if (next == InputActionPhase::Canceled || next == InputActionPhase::Completed || next == InputActionPhase::Started ||
			    next == InputActionPhase::Ongoing)
				return next;
			return current;
		}
	}// namespace

	InputValue InputModifier::apply(const InputEvent&, const InputValue& value) const
	{
		return value;
	}

	InputValue InputModifierScale::apply(const InputEvent&, const InputValue& value) const
	{
		InputValue result = value;
		result.axes *= m_scale;
		return result;
	}

	InputValue InputModifierDeadZone::apply(const InputEvent&, const InputValue& value) const
	{
		InputValue result = value;
		if (value_length_squared(result) < m_threshold * m_threshold)
			result.axes = {0.f, 0.f, 0.f};
		return result;
	}

	InputValue InputModifierInvert::apply(const InputEvent&, const InputValue& value) const
	{
		InputValue result = value;
		if (m_x)
			result.axes.x = -result.axes.x;
		if (m_y)
			result.axes.y = -result.axes.y;
		if (m_z)
			result.axes.z = -result.axes.z;
		return result;
	}

	InputValue InputModifierClamp::apply(const InputEvent&, const InputValue& value) const
	{
		InputValue result = value;
		result.axes       = {clamp(result.axes.x, m_min.x, m_max.x), clamp(result.axes.y, m_min.y, m_max.y),
		                     clamp(result.axes.z, m_min.z, m_max.z)};
		return result;
	}

	InputValue InputModifierSwizzle::apply(const InputEvent&, const InputValue& value) const
	{
		InputValue result = value;
		result.axes       = {component(value.axes, m_x), component(value.axes, m_y), component(value.axes, m_z)};
		return result;
	}

	InputActionPhase InputTriggerPressed::evaluate(const InputEvent& event, const InputValue&, InputTriggerState&) const
	{
		return event.type == EventType::Press ? InputActionPhase::Triggered : InputActionPhase::None;
	}

	InputActionPhase InputTriggerReleased::evaluate(const InputEvent& event, const InputValue&, InputTriggerState&) const
	{
		return event.type == EventType::Release ? InputActionPhase::Triggered : InputActionPhase::None;
	}

	InputActionPhase InputTriggerDown::evaluate(const InputEvent&, const InputValue& value, InputTriggerState& state) const
	{
		state.active = value.is_active();
		state.value  = value;
		return state.active ? InputActionPhase::Started : InputActionPhase::Completed;
	}

	InputActionPhase InputTriggerDown::update(f32, InputTriggerState& state) const
	{
		return state.active ? InputActionPhase::Ongoing : InputActionPhase::None;
	}

	InputActionPhase InputTriggerHold::evaluate(const InputEvent&, const InputValue& value, InputTriggerState& state) const
	{
		const bool active = value.is_active();
		if (active && !state.active)
		{
			state.active  = true;
			state.elapsed = 0.f;
			state.value   = value;
			return InputActionPhase::Started;
		}
		if (!active && state.active)
		{
			const bool triggered = state.elapsed >= m_duration;
			state.active         = false;
			return triggered ? InputActionPhase::Completed : InputActionPhase::Canceled;
		}
		state.value = value;
		return InputActionPhase::None;
	}

	InputActionPhase InputTriggerHold::update(f32 dt, InputTriggerState& state) const
	{
		if (!state.active)
			return InputActionPhase::None;
		const f32 previous = state.elapsed;
		state.elapsed += dt;
		return previous < m_duration && state.elapsed >= m_duration ? InputActionPhase::Triggered : InputActionPhase::Ongoing;
	}

	InputActionPhase InputTriggerThreshold::evaluate(const InputEvent&, const InputValue& value, InputTriggerState& state) const
	{
		state.active = value_length_squared(value) >= m_threshold * m_threshold;
		state.value  = value;
		return state.active ? InputActionPhase::Triggered : InputActionPhase::None;
	}

	InputActionPhase InputTriggerThreshold::update(f32, InputTriggerState& state) const
	{
		return state.active ? InputActionPhase::Ongoing : InputActionPhase::None;
	}

	InputActionPhase InputTriggerTap::evaluate(const InputEvent&, const InputValue& value, InputTriggerState& state) const
	{
		if (value.is_active() && !state.active)
		{
			state.active  = true;
			state.elapsed = 0.f;
			state.value   = value;
			return InputActionPhase::Started;
		}

		if (!value.is_active() && state.active)
		{
			state.active = false;
			return state.elapsed <= m_max_duration ? InputActionPhase::Triggered : InputActionPhase::Canceled;
		}

		state.value = value;
		return InputActionPhase::None;
	}

	InputActionPhase InputTriggerTap::update(f32 dt, InputTriggerState& state) const
	{
		if (!state.active)
			return InputActionPhase::None;

		state.elapsed += dt;
		return state.elapsed <= m_max_duration ? InputActionPhase::Ongoing : InputActionPhase::None;
	}

	InputActionPhase InputTriggerChord::evaluate(const InputEvent& event, const InputValue& value, InputTriggerState& state) const
	{
		if (!event.device || m_codes.empty())
			return InputActionPhase::None;

		for (EventCode code : m_codes)
		{
			bool active              = event.code == code ? is_active(event) : false;
			u32 count                = 0;
			const InputEvent* events = event.device->state(count);
			for (u32 index = 0; event.code != code && !active && events && index < count; ++index)
			{
				active = events[index].code == code && is_active(events[index]);
			}

			if (!active)
			{
				const bool was_active = state.active;
				state.active          = false;
				return was_active ? InputActionPhase::Completed : InputActionPhase::None;
			}
		}

		const bool was_active = state.active;
		state.active          = true;
		state.value           = value;
		return was_active ? InputActionPhase::Ongoing : InputActionPhase::Triggered;
	}

	InputActionPhase InputTriggerAll::evaluate(const InputEvent& event, const InputValue& value, InputTriggerState& state) const
	{
		if (m_triggers.empty())
			return InputActionPhase::None;
		state.children.resize(m_triggers.size());
		InputActionPhase result = InputActionPhase::Triggered;
		for (usize index = 0; index < m_triggers.size(); ++index)
		{
			if (!m_triggers[index])
				return InputActionPhase::None;
			const InputActionPhase phase = m_triggers[index]->evaluate(event, value, state.children[index]);
			if (phase == InputActionPhase::None)
				return InputActionPhase::None;
			result = combine_phase(result, phase);
		}
		state.value = value;
		return result;
	}

	InputActionPhase InputTriggerAll::update(f32 dt, InputTriggerState& state) const
	{
		if (state.children.size() != m_triggers.size())
			return InputActionPhase::None;
		InputActionPhase result = InputActionPhase::Triggered;
		for (usize index = 0; index < m_triggers.size(); ++index)
		{
			if (!m_triggers[index])
				return InputActionPhase::None;
			const InputActionPhase phase = m_triggers[index]->update(dt, state.children[index]);
			if (phase == InputActionPhase::None)
				return InputActionPhase::None;
			result = combine_phase(result, phase);
		}
		return result;
	}

	InputActionPhase InputTriggerAny::evaluate(const InputEvent& event, const InputValue& value, InputTriggerState& state) const
	{
		state.children.resize(m_triggers.size());
		InputActionPhase result = InputActionPhase::None;
		for (usize index = 0; index < m_triggers.size(); ++index)
		{
			if (m_triggers[index])
			{
				const InputActionPhase phase = m_triggers[index]->evaluate(event, value, state.children[index]);
				if (phase != InputActionPhase::None)
					result = combine_phase(result, phase);
			}
		}
		state.value = value;
		return result;
	}

	InputActionPhase InputTriggerAny::update(f32 dt, InputTriggerState& state) const
	{
		if (state.children.size() != m_triggers.size())
			return InputActionPhase::None;
		InputActionPhase result = InputActionPhase::None;
		for (usize index = 0; index < m_triggers.size(); ++index)
		{
			if (m_triggers[index])
			{
				const InputActionPhase phase = m_triggers[index]->update(dt, state.children[index]);
				if (phase != InputActionPhase::None)
					result = combine_phase(result, phase);
			}
		}
		return result;
	}

	InputContext& InputContext::add_mapping(const InputMapping& mapping)
	{
		if (mapping.action)
		{
			m_mappings.push_back(mapping);
			++m_revision;
		}
		return *this;
	}

	InputContext& InputContext::enabled(bool value)
	{
		m_enabled = value;
		return *this;
	}

	InputContext& InputContext::remove_mapping(const InputAction* action)
	{
		for (auto it = m_mappings.begin(); it != m_mappings.end();)
		{
			if (it->action == action)
			{
				it = m_mappings.erase(it);
				++m_revision;
			}
			else
			{
				++it;
			}
		}
		return *this;
	}
}// namespace Trinex
