#include <SDLPlatform/input.hpp>

#include <Core/etl/algorithm.hpp>
#include <Input/device.hpp>
#include <Input/system.hpp>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_touch.h>


namespace Trinex::Platform
{
	namespace
	{
		enum class DeviceClass
		{
			Any,
			Keyboard,
			Mouse,
			Gamepad,
		};

		DeviceClass device_class(const InputDeviceType& type)
		{
			if (type == InputDeviceType::Keyboard)
				return DeviceClass::Keyboard;
			if (type == InputDeviceType::Mouse)
				return DeviceClass::Mouse;
			return DeviceClass::Gamepad;
		}

		EventCode key_code(SDL_Scancode code)
		{
			static const EventCode* const letters[] = {
			        &EventCode::KeyA, &EventCode::KeyB, &EventCode::KeyC, &EventCode::KeyD, &EventCode::KeyE, &EventCode::KeyF,
			        &EventCode::KeyG, &EventCode::KeyH, &EventCode::KeyI, &EventCode::KeyJ, &EventCode::KeyK, &EventCode::KeyL,
			        &EventCode::KeyM, &EventCode::KeyN, &EventCode::KeyO, &EventCode::KeyP, &EventCode::KeyQ, &EventCode::KeyR,
			        &EventCode::KeyS, &EventCode::KeyT, &EventCode::KeyU, &EventCode::KeyV, &EventCode::KeyW, &EventCode::KeyX,
			        &EventCode::KeyY, &EventCode::KeyZ};
			static const EventCode* const digits[] = {&EventCode::Key1, &EventCode::Key2, &EventCode::Key3, &EventCode::Key4,
			                                          &EventCode::Key5, &EventCode::Key6, &EventCode::Key7, &EventCode::Key8,
			                                          &EventCode::Key9, &EventCode::Key0};

			if (code >= SDL_SCANCODE_A && code <= SDL_SCANCODE_Z)
				return *letters[code - SDL_SCANCODE_A];
			if (code >= SDL_SCANCODE_1 && code <= SDL_SCANCODE_0)
				return *digits[code - SDL_SCANCODE_1];

			switch (code)
			{
				case SDL_SCANCODE_RETURN: return EventCode::KeyEnter;
				case SDL_SCANCODE_ESCAPE: return EventCode::KeyEscape;
				case SDL_SCANCODE_BACKSPACE: return EventCode::KeyBackspace;
				case SDL_SCANCODE_TAB: return EventCode::KeyTab;
				case SDL_SCANCODE_SPACE: return EventCode::KeySpace;
				case SDL_SCANCODE_LEFT: return EventCode::KeyLeft;
				case SDL_SCANCODE_RIGHT: return EventCode::KeyRight;
				case SDL_SCANCODE_UP: return EventCode::KeyUp;
				case SDL_SCANCODE_DOWN: return EventCode::KeyDown;
				case SDL_SCANCODE_LSHIFT: return EventCode::KeyLeftShift;
				case SDL_SCANCODE_RSHIFT: return EventCode::KeyRightShift;
				case SDL_SCANCODE_LCTRL: return EventCode::KeyLeftControl;
				case SDL_SCANCODE_RCTRL: return EventCode::KeyRightControl;
				case SDL_SCANCODE_LALT: return EventCode::KeyLeftAlt;
				case SDL_SCANCODE_RALT: return EventCode::KeyRightAlt;
				case SDL_SCANCODE_LGUI: return EventCode::KeyLeftSuper;
				case SDL_SCANCODE_RGUI: return EventCode::KeyRightSuper;
				case SDL_SCANCODE_DELETE: return EventCode::KeyDelete;
				case SDL_SCANCODE_INSERT: return EventCode::KeyInsert;
				case SDL_SCANCODE_HOME: return EventCode::KeyHome;
				case SDL_SCANCODE_END: return EventCode::KeyEnd;
				case SDL_SCANCODE_PAGEUP: return EventCode::KeyPageUp;
				case SDL_SCANCODE_PAGEDOWN: return EventCode::KeyPageDown;
				default: return EventCode::Undefined;
			}
		}

		EventCode mouse_button_code(Uint8 button)
		{
			switch (button)
			{
				case SDL_BUTTON_LEFT: return EventCode::MouseLeft;
				case SDL_BUTTON_RIGHT: return EventCode::MouseRight;
				case SDL_BUTTON_MIDDLE: return EventCode::MouseMiddle;
				case SDL_BUTTON_X1: return EventCode::MouseBack;
				case SDL_BUTTON_X2: return EventCode::MouseForward;
				default: return EventCode::Undefined;
			}
		}

		EventCode gamepad_button_code(SDL_GamepadButton button)
		{
			switch (button)
			{
				case SDL_GAMEPAD_BUTTON_SOUTH: return EventCode::GamepadFaceBottom;
				case SDL_GAMEPAD_BUTTON_EAST: return EventCode::GamepadFaceRight;
				case SDL_GAMEPAD_BUTTON_WEST: return EventCode::GamepadFaceLeft;
				case SDL_GAMEPAD_BUTTON_NORTH: return EventCode::GamepadFaceTop;
				case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return EventCode::GamepadLeftShoulder;
				case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return EventCode::GamepadRightShoulder;
				case SDL_GAMEPAD_BUTTON_LEFT_STICK: return EventCode::GamepadLeftStickButton;
				case SDL_GAMEPAD_BUTTON_RIGHT_STICK: return EventCode::GamepadRightStickButton;
				case SDL_GAMEPAD_BUTTON_DPAD_UP: return EventCode::GamepadDPadUp;
				case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return EventCode::GamepadDPadDown;
				case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return EventCode::GamepadDPadLeft;
				case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return EventCode::GamepadDPadRight;
				case SDL_GAMEPAD_BUTTON_START: return EventCode::GamepadStart;
				case SDL_GAMEPAD_BUTTON_BACK: return EventCode::GamepadBack;
				case SDL_GAMEPAD_BUTTON_GUIDE: return EventCode::GamepadGuide;
				default: return EventCode::Undefined;
			}
		}

		EventCode gamepad_axis_code(SDL_GamepadAxis axis)
		{
			switch (axis)
			{
				case SDL_GAMEPAD_AXIS_LEFTX: return EventCode::GamepadLeftX;
				case SDL_GAMEPAD_AXIS_LEFTY: return EventCode::GamepadLeftY;
				case SDL_GAMEPAD_AXIS_RIGHTX: return EventCode::GamepadRightX;
				case SDL_GAMEPAD_AXIS_RIGHTY: return EventCode::GamepadRightY;
				case SDL_GAMEPAD_AXIS_LEFT_TRIGGER: return EventCode::GamepadLeftTrigger;
				case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER: return EventCode::GamepadRightTrigger;
				default: return EventCode::Undefined;
			}
		}
	}// namespace

	SDLInputDevice::SDLInputDevice(InputDeviceType type, u64 id, String name) : m_type(type), m_id(id), m_name(etl::move(name)) {}

	const char* SDLInputDevice::name() const
	{
		return m_name.c_str();
	}

	u32 SDLInputDevice::types() const
	{
		return 1;
	}

	InputDeviceType SDLInputDevice::type(u32 index) const
	{
		return index == 0 ? m_type : InputDeviceType::Undefined;
	}

	const InputEvent* SDLInputDevice::state(u32& len) const
	{
		len = static_cast<u32>(m_state.size());
		return m_state.data();
	}

	u64 SDLInputDevice::id() const
	{
		return m_id;
	}

	void SDLInputDevice::emit(InputEvent event)
	{
		event.device = this;
		if (event.code)
		{
			auto it = etl::find_if(m_state.begin(), m_state.end(),
			                       [&event](const InputEvent& state) { return state.code == event.code; });
			if (it == m_state.end())
				m_state.push_back(event);
			else
				*it = event;
		}

		on_event(event);
	}

	SDLInputSystem* SDLInputSystem::instance()
	{
		static SDLInputSystem input;
		return &input;
	}

	void SDLInputSystem::update()
	{
		auto find_device = [this](DeviceClass type, u64 id) -> SDLInputDevice* {
			for (const Ref<SDLInputDevice>& device : m_devices)
			{
				if (device && device_class(device->type()) == type && device->id() == id)
					return device.value();
			}
			return nullptr;
		};

		auto add_device = [this, &find_device](InputDeviceType type, u64 id, const char* name) {
			if (find_device(device_class(type), id))
				return;

			Ref<SDLInputDevice> device = Ref<SDLInputDevice>::make(type, id, name ? name : "SDL input device");
			m_devices.push_back(device);
			if (::Trinex::InputSystem* input = ::Trinex::InputSystem::instance())
				input->add_device(device);

			InputEvent input_event;
			input_event.type = EventType::DeviceAdded;
			device->emit(input_event);
		};

		auto remove_device = [this, &find_device](DeviceClass type, u64 id) {
			SDLInputDevice* device = find_device(type, id);
			if (!device)
				return;

			InputEvent input_event;
			input_event.type = EventType::DeviceRemoved;
			device->emit(input_event);
			if (::Trinex::InputSystem* input = ::Trinex::InputSystem::instance())
				input->remove_device(device);

			for (auto it = m_devices.begin(); it != m_devices.end(); ++it)
			{
				if (it->value() == device)
				{
					m_devices.erase(it);
					break;
				}
			}
		};

		auto route_event = [this, &find_device](InputEvent input_event, DeviceClass type, u64 id) {
			if (type != DeviceClass::Any)
			{
				if (SDLInputDevice* device = find_device(type, id))
				{
					device->emit(input_event);
					return;
				}

				for (const Ref<SDLInputDevice>& device : m_devices)
				{
					if (device && device_class(device->type()) == type)
						device->emit(input_event);
				}
				return;
			}

			for (const Ref<SDLInputDevice>& device : m_devices)
			{
				if (device)
					device->emit(input_event);
			}
		};

		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_EVENT_KEYBOARD_ADDED:
					add_device(InputDeviceType::Keyboard, event.kdevice.which, SDL_GetKeyboardNameForID(event.kdevice.which));
					break;
				case SDL_EVENT_KEYBOARD_REMOVED: remove_device(DeviceClass::Keyboard, event.kdevice.which); break;
				case SDL_EVENT_MOUSE_ADDED:
					add_device(InputDeviceType::Mouse, event.mdevice.which, SDL_GetMouseNameForID(event.mdevice.which));
					break;
				case SDL_EVENT_MOUSE_REMOVED: remove_device(DeviceClass::Mouse, event.mdevice.which); break;
				case SDL_EVENT_JOYSTICK_ADDED:
					if (!SDL_IsGamepad(event.jdevice.which))
						add_device(InputDeviceType::Gamepad, event.jdevice.which, SDL_GetJoystickNameForID(event.jdevice.which));
					break;
				case SDL_EVENT_JOYSTICK_REMOVED:
					if (!SDL_IsGamepad(event.jdevice.which))
						remove_device(DeviceClass::Gamepad, event.jdevice.which);
					break;
				case SDL_EVENT_GAMEPAD_ADDED:
					add_device(InputDeviceType::Gamepad, event.gdevice.which, SDL_GetGamepadNameForID(event.gdevice.which));
					break;
				case SDL_EVENT_GAMEPAD_REMOVED: remove_device(DeviceClass::Gamepad, event.gdevice.which); break;
				case SDL_EVENT_KEY_DOWN:
				case SDL_EVENT_KEY_UP:
				{
					InputEvent input_event;
					input_event.window_id = event.key.windowID;
					input_event.code      = key_code(event.key.scancode);
					input_event.type      = event.key.repeat ? EventType::Repeat
					                        : event.key.down ? EventType::Press
					                                         : EventType::Release;
					input_event.value     = InputValue::boolean(event.key.down);
					route_event(input_event, DeviceClass::Keyboard, event.key.which);
					break;
				}
				case SDL_EVENT_TEXT_EDITING:
				case SDL_EVENT_TEXT_INPUT:
				{
					InputEvent input_event;
					input_event.window_id  = event.type == SDL_EVENT_TEXT_EDITING ? event.edit.windowID : event.text.windowID;
					input_event.type       = event.type == SDL_EVENT_TEXT_EDITING ? EventType::TextEditing : EventType::TextInput;
					input_event.code       = EventCode::Text;
					input_event.value.type = InputValueType{InputValueType::Text};
					input_event.payload<const char*>() = event.type == SDL_EVENT_TEXT_EDITING ? event.edit.text : event.text.text;
					route_event(input_event, DeviceClass::Keyboard, 0);
					break;
				}
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
				case SDL_EVENT_MOUSE_BUTTON_UP:
				{
					InputEvent input_event;
					input_event.window_id = event.button.windowID;
					input_event.type      = event.button.down ? EventType::Press : EventType::Release;
					input_event.code      = mouse_button_code(event.button.button);
					input_event.value     = InputValue::boolean(event.button.down);
					route_event(input_event, DeviceClass::Mouse, event.button.which);
					break;
				}
				case SDL_EVENT_MOUSE_MOTION:
				{
					InputEvent input_event;
					input_event.window_id = event.motion.windowID;
					input_event.type      = EventType::Axis;
					input_event.code      = EventCode::MouseX;
					input_event.value     = InputValue::axis_1d(event.motion.x);
					route_event(input_event, DeviceClass::Mouse, event.motion.which);
					input_event.code  = EventCode::MouseY;
					input_event.value = InputValue::axis_1d(event.motion.y);
					route_event(input_event, DeviceClass::Mouse, event.motion.which);
					input_event.code  = EventCode::MouseDeltaX;
					input_event.value = InputValue::axis_1d(event.motion.xrel);
					route_event(input_event, DeviceClass::Mouse, event.motion.which);
					input_event.code  = EventCode::MouseDeltaY;
					input_event.value = InputValue::axis_1d(event.motion.yrel);
					route_event(input_event, DeviceClass::Mouse, event.motion.which);
					break;
				}
				case SDL_EVENT_MOUSE_WHEEL:
				{
					InputEvent input_event;
					input_event.window_id = event.wheel.windowID;
					input_event.type      = EventType::Axis;
					input_event.code      = EventCode::MouseWheelX;
					input_event.value     = InputValue::axis_1d(event.wheel.x);
					route_event(input_event, DeviceClass::Mouse, event.wheel.which);
					input_event.code  = EventCode::MouseWheelY;
					input_event.value = InputValue::axis_1d(event.wheel.y);
					route_event(input_event, DeviceClass::Mouse, event.wheel.which);
					break;
				}
				case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
				case SDL_EVENT_GAMEPAD_BUTTON_UP:
				{
					InputEvent input_event;
					input_event.type  = event.gbutton.down ? EventType::Press : EventType::Release;
					input_event.code  = gamepad_button_code(static_cast<SDL_GamepadButton>(event.gbutton.button));
					input_event.value = InputValue::boolean(event.gbutton.down);
					route_event(input_event, DeviceClass::Gamepad, event.gbutton.which);
					break;
				}
				case SDL_EVENT_GAMEPAD_AXIS_MOTION:
				{
					InputEvent input_event;
					input_event.type  = EventType::Axis;
					input_event.code  = gamepad_axis_code(static_cast<SDL_GamepadAxis>(event.gaxis.axis));
					input_event.value = InputValue::axis_1d(static_cast<f32>(event.gaxis.value) / 32767.f);
					route_event(input_event, DeviceClass::Gamepad, event.gaxis.which);
					break;
				}
				default:
				{
					InputEvent input_event;
					route_event(input_event, DeviceClass::Any, 0);
					break;
				}
			}
		}
	}

	usize SDLInputSystem::devices() const
	{
		return m_devices.size();
	}

	WeakRef<InputDevice> SDLInputSystem::device(usize idx) const
	{
		return idx < m_devices.size() ? m_devices[idx] : nullptr;
	}

	bool SDLInputSystem::text_input_enabled() const
	{
		return SDL_TextInputActive(nullptr);
	}

	InputSystem& SDLInputSystem::text_input_enabled(bool enabled)
	{
		if (enabled)
			SDL_StartTextInput(nullptr);
		else
			SDL_StopTextInput(nullptr);
		return *this;
	}
}// namespace Trinex::Platform
