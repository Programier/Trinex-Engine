#pragma once
#include <Core/etl/delegate.hpp>
#include <Core/etl/vector.hpp>
#include <Core/ref_counted.hpp>
#include <Input/types.hpp>

namespace Trinex
{
	class InputDevice;

	struct InputActionPhase {
		enum Enum : u8
		{
			None,
			Started,
			Ongoing,
			Triggered,
			Completed,
			Canceled
		};
		trinex_enum_struct(InputActionPhase);
	};

	class ENGINE_EXPORT InputAction : public RefCounted
	{
		Name m_name;
		InputValueType m_value_type;

	public:
		InputAction(Name name, InputValueType value_type) : m_name(etl::move(name)), m_value_type(value_type) {}
		const Name& name() const { return m_name; }
		InputValueType value_type() const { return m_value_type; }
	};

	struct InputControl {
		InputDeviceType device_type = InputDeviceType::Undefined;
		u32 device_id               = 0;
		EventType type              = EventType::Undefined;
		EventCode code              = EventCode::Undefined;
	};

	class ENGINE_EXPORT InputModifier : public RefCounted
	{
	public:
		virtual InputValue apply(const InputEvent& event, const InputValue& value) const;
	};

	class ENGINE_EXPORT InputModifierScale final : public InputModifier
	{
		Vector3f m_scale;

	public:
		explicit InputModifierScale(const Vector3f& scale) : m_scale(scale) {}
		InputValue apply(const InputEvent& event, const InputValue& value) const override;
	};

	class ENGINE_EXPORT InputModifierDeadZone final : public InputModifier
	{
		f32 m_threshold;

	public:
		explicit InputModifierDeadZone(f32 threshold) : m_threshold(threshold) {}
		InputValue apply(const InputEvent& event, const InputValue& value) const override;
	};

	class ENGINE_EXPORT InputModifierInvert final : public InputModifier
	{
		bool m_x;
		bool m_y;
		bool m_z;

	public:
		InputModifierInvert(bool x = true, bool y = true, bool z = true) : m_x(x), m_y(y), m_z(z) {}
		InputValue apply(const InputEvent& event, const InputValue& value) const override;
	};

	class ENGINE_EXPORT InputModifierClamp final : public InputModifier
	{
		Vector3f m_min;
		Vector3f m_max;

	public:
		InputModifierClamp(const Vector3f& min, const Vector3f& max) : m_min(min), m_max(max) {}
		InputValue apply(const InputEvent& event, const InputValue& value) const override;
	};

	class ENGINE_EXPORT InputModifierSwizzle final : public InputModifier
	{
		u8 m_x;
		u8 m_y;
		u8 m_z;

	public:
		InputModifierSwizzle(u8 x, u8 y, u8 z) : m_x(x), m_y(y), m_z(z) {}
		InputValue apply(const InputEvent& event, const InputValue& value) const override;
	};

	struct InputTriggerState {
		bool active      = false;
		f32 elapsed      = 0.f;
		InputValue value = {};
		Vector<InputTriggerState> children;
	};

	class ENGINE_EXPORT InputTrigger : public RefCounted
	{
	public:
		virtual InputActionPhase evaluate(const InputEvent& event, const InputValue& value, InputTriggerState& state) const = 0;
		virtual InputActionPhase update(f32, InputTriggerState&) const { return InputActionPhase::None; }
	};

	class ENGINE_EXPORT InputTriggerPressed final : public InputTrigger
	{
	public:
		InputActionPhase evaluate(const InputEvent& event, const InputValue& value, InputTriggerState& state) const override;
	};

	class ENGINE_EXPORT InputTriggerReleased final : public InputTrigger
	{
	public:
		InputActionPhase evaluate(const InputEvent& event, const InputValue& value, InputTriggerState& state) const override;
	};

	class ENGINE_EXPORT InputTriggerDown final : public InputTrigger
	{
	public:
		InputActionPhase evaluate(const InputEvent& event, const InputValue& value, InputTriggerState& state) const override;
		InputActionPhase update(f32 dt, InputTriggerState& state) const override;
	};

	class ENGINE_EXPORT InputTriggerHold final : public InputTrigger
	{
		f32 m_duration;

	public:
		explicit InputTriggerHold(f32 duration) : m_duration(duration) {}
		InputActionPhase evaluate(const InputEvent& event, const InputValue& value, InputTriggerState& state) const override;
		InputActionPhase update(f32 dt, InputTriggerState& state) const override;
	};

	class ENGINE_EXPORT InputTriggerThreshold final : public InputTrigger
	{
		f32 m_threshold;

	public:
		explicit InputTriggerThreshold(f32 threshold) : m_threshold(threshold) {}
		InputActionPhase evaluate(const InputEvent& event, const InputValue& value, InputTriggerState& state) const override;
		InputActionPhase update(f32 dt, InputTriggerState& state) const override;
	};

	class ENGINE_EXPORT InputTriggerTap final : public InputTrigger
	{
		f32 m_max_duration;

	public:
		explicit InputTriggerTap(f32 max_duration) : m_max_duration(max_duration) {}
		InputActionPhase evaluate(const InputEvent& event, const InputValue& value, InputTriggerState& state) const override;
		InputActionPhase update(f32 dt, InputTriggerState& state) const override;
	};

	class ENGINE_EXPORT InputTriggerChord final : public InputTrigger
	{
		Vector<EventCode> m_codes;

	public:
		explicit InputTriggerChord(const Vector<EventCode>& codes) : m_codes(codes) {}
		InputActionPhase evaluate(const InputEvent& event, const InputValue& value, InputTriggerState& state) const override;
	};

	class ENGINE_EXPORT InputTriggerAll final : public InputTrigger
	{
		Vector<Ref<InputTrigger>> m_triggers;

	public:
		explicit InputTriggerAll(const Vector<Ref<InputTrigger>>& triggers) : m_triggers(triggers) {}
		InputActionPhase evaluate(const InputEvent& event, const InputValue& value, InputTriggerState& state) const override;
		InputActionPhase update(f32 dt, InputTriggerState& state) const override;
	};

	class ENGINE_EXPORT InputTriggerAny final : public InputTrigger
	{
		Vector<Ref<InputTrigger>> m_triggers;

	public:
		explicit InputTriggerAny(const Vector<Ref<InputTrigger>>& triggers) : m_triggers(triggers) {}
		InputActionPhase evaluate(const InputEvent& event, const InputValue& value, InputTriggerState& state) const override;
		InputActionPhase update(f32 dt, InputTriggerState& state) const override;
	};

	struct InputMapping {
		Ref<InputAction> action;
		InputControl control;
		Vector<Ref<InputModifier>> modifiers;
		Vector<Ref<InputTrigger>> triggers;
		bool consume              = false;
		bool block_lower_priority = false;
	};

	class ENGINE_EXPORT InputContext : public RefCounted
	{
		Name m_name;
		Vector<InputMapping> m_mappings;
		u64 m_revision = 0;
		bool m_enabled = true;

	public:
		explicit InputContext(Name name) : m_name(etl::move(name)) {}
		const Name& name() const { return m_name; }
		u64 revision() const { return m_revision; }
		bool enabled() const { return m_enabled; }
		InputContext& enabled(bool value);
		InputContext& add_mapping(const InputMapping& mapping);
		InputContext& remove_mapping(const InputAction* action);
		const Vector<InputMapping>& mappings() const { return m_mappings; }
	};

	struct InputActionEvent {
		Ref<InputAction> action;
		InputValue value;
		InputActionPhase phase      = InputActionPhase::None;
		InputDevice* device         = nullptr;
		const InputContext* context = nullptr;
	};

	struct InputActionState {
		Ref<InputAction> action;
		InputValue value;
		InputActionPhase phase    = InputActionPhase::None;
		bool triggered_this_frame = false;
	};

	using InputActionListenerHandle = Handle<class InputActionListenerTag>;
	using InputActionListener       = Delegate<bool(const InputActionEvent&)>;

}// namespace Trinex
