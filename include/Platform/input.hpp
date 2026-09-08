#pragma once
#include <Core/etl/vector.hpp>
#include <Input/input_codes.hpp>

namespace Trinex
{
	struct InputDevice;
	struct InputDeviceState;
}// namespace Trinex

namespace Trinex::Platform
{
	class ENGINE_EXPORT InputSystem
	{
	public:
		static InputSystem* instance();

		virtual ~InputSystem() = default;

		virtual bool devices(Vector<InputDevice>* out) const                                                        = 0;
		virtual bool device_state(DeviceId device_id, InputDeviceState* out) const                                  = 0;
		virtual bool set_gamepad_rumble(DeviceId device_id, f32 low_frequency, f32 high_frequency, u32 duration_ms) = 0;
		virtual bool text_input_enabled() const                                                                     = 0;
		virtual InputSystem* text_input_enabled(bool enabled)                                                       = 0;
	};
}// namespace Trinex::Platform
