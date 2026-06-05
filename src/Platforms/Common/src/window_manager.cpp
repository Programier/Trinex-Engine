#include <Core/base_engine.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/threading.hpp>
#include <Input/event_system.hpp>
#include <Input/input_system.hpp>
#include <Platform/platform.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <sdl_window.hpp>

namespace Trinex::Platform
{
	static Map<SDL_JoystickID, SDL_GameController*> m_game_controllers;

	ENGINE_EXPORT usize monitors_count()
	{
		return static_cast<usize>(SDL_GetNumVideoDisplays());
	}

	ENGINE_EXPORT MonitorInfo monitor_info(usize monitor_index)
	{
		MonitorInfo info;
		SDL_Rect r;
		SDL_GetDisplayBounds(monitor_index, &r);
		info.pos  = {r.x, r.y};
		info.size = {r.w, r.h};
		SDL_GetDisplayDPI(monitor_index, &info.dpi, nullptr, nullptr);

		if (monitor_index != 0)
		{
			MonitorInfo main_monitor_info = monitor_info(0);
			info.pos.y                    = main_monitor_info.size.y - (info.pos.y + info.size.y);
		}
		return info;
	}

	namespace WindowManager
	{
		ENGINE_EXPORT void initialize()
		{
			SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_SENSOR);
		}

		ENGINE_EXPORT void terminate()
		{
			for (auto& pair : m_game_controllers)
			{
				info_log("WindowSDL", "Force close controller with id %d", pair.first);
				SDL_GameControllerClose(pair.second);
			}

			m_game_controllers.clear();

			SDL_Quit();
		}

		ENGINE_EXPORT Window* create_window(const WindowConfig* config)
		{
			return (new WindowSDL())->sdl_initialize(config);
		}

		ENGINE_EXPORT void destroy_window(Window* window)
		{
			delete window;
		}

		ENGINE_EXPORT void mouse_relative_mode(bool flag)
		{
			SDL_SetRelativeMouseMode(static_cast<SDL_bool>(flag));
		}
	}// namespace WindowManager

	namespace EventSystem
	{
		static SDL_Event m_event;

		namespace
		{
			static DeviceId make_window_device_id(Uint32 window_id, InputDeviceType type)
			{
				return (static_cast<DeviceId>(type) << 56) | static_cast<DeviceId>(window_id);
			}

			static DeviceId make_gamepad_device_id(SDL_JoystickID joystick_id)
			{
				return (static_cast<DeviceId>(InputDeviceType::Gamepad) << 56) |
				       static_cast<DeviceId>(static_cast<u32>(joystick_id));
			}

			static MouseButton map_mouse_button(Uint8 button)
			{
				switch (button)
				{
					case SDL_BUTTON_LEFT: return MouseButton::Left;
					case SDL_BUTTON_RIGHT: return MouseButton::Right;
					case SDL_BUTTON_MIDDLE: return MouseButton::Middle;
					case SDL_BUTTON_X1: return MouseButton::X1;
					case SDL_BUTTON_X2: return MouseButton::X2;
					default: return MouseButton::Undefined;
				}
			}

			static GamepadAxis map_gamepad_axis(Uint8 axis)
			{
				switch (axis)
				{
					case SDL_CONTROLLER_AXIS_LEFTX: return GamepadAxis::LeftX;
					case SDL_CONTROLLER_AXIS_LEFTY: return GamepadAxis::LeftY;
					case SDL_CONTROLLER_AXIS_RIGHTX: return GamepadAxis::RightX;
					case SDL_CONTROLLER_AXIS_RIGHTY: return GamepadAxis::RightY;
					case SDL_CONTROLLER_AXIS_TRIGGERLEFT: return GamepadAxis::LeftTrigger;
					case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: return GamepadAxis::RightTrigger;
					default: return GamepadAxis::Undefined;
				}
			}

			static GamepadButton map_gamepad_button(Uint8 button)
			{
				switch (button)
				{
					case SDL_CONTROLLER_BUTTON_A: return GamepadButton::FaceBottom;
					case SDL_CONTROLLER_BUTTON_B: return GamepadButton::FaceRight;
					case SDL_CONTROLLER_BUTTON_X: return GamepadButton::FaceLeft;
					case SDL_CONTROLLER_BUTTON_Y: return GamepadButton::FaceTop;
					case SDL_CONTROLLER_BUTTON_BACK: return GamepadButton::Select;
					case SDL_CONTROLLER_BUTTON_GUIDE: return GamepadButton::Guide;
					case SDL_CONTROLLER_BUTTON_START: return GamepadButton::Start;
					case SDL_CONTROLLER_BUTTON_LEFTSTICK: return GamepadButton::LeftStick;
					case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return GamepadButton::RightStick;
					case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return GamepadButton::LeftShoulder;
					case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return GamepadButton::RightShoulder;
					case SDL_CONTROLLER_BUTTON_DPAD_UP: return GamepadButton::DPadUp;
					case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return GamepadButton::DPadDown;
					case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return GamepadButton::DPadLeft;
					case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return GamepadButton::DPadRight;
#ifdef SDL_CONTROLLER_BUTTON_MISC1
					case SDL_CONTROLLER_BUTTON_MISC1: return GamepadButton::Misc1;
#endif
#ifdef SDL_CONTROLLER_BUTTON_PADDLE1
					case SDL_CONTROLLER_BUTTON_PADDLE1: return GamepadButton::Paddle1;
					case SDL_CONTROLLER_BUTTON_PADDLE2: return GamepadButton::Paddle2;
					case SDL_CONTROLLER_BUTTON_PADDLE3: return GamepadButton::Paddle3;
					case SDL_CONTROLLER_BUTTON_PADDLE4: return GamepadButton::Paddle4;
#endif
#ifdef SDL_CONTROLLER_BUTTON_TOUCHPAD
					case SDL_CONTROLLER_BUTTON_TOUCHPAD: return GamepadButton::Touchpad;
#endif
					default: return GamepadButton::Undefined;
				}
			}

			static i32 window_height(Uint32 window_id)
			{
				if (SDL_Window* window = SDL_GetWindowFromID(window_id))
				{
					int width  = 0;
					int height = 0;
					SDL_GetWindowSize(window, &width, &height);
					return height;
				}

				return 0;
			}

			static char32_t decode_utf8_codepoint(const char*& text)
			{
				const u8 first = static_cast<u8>(*text++);

				if ((first & 0x80u) == 0)
				{
					return static_cast<char32_t>(first);
				}

				if ((first & 0xE0u) == 0xC0u)
				{
					const u8 second = static_cast<u8>(*text++);
					return static_cast<char32_t>(((first & 0x1Fu) << 6) | (second & 0x3Fu));
				}

				if ((first & 0xF0u) == 0xE0u)
				{
					const u8 second = static_cast<u8>(*text++);
					const u8 third  = static_cast<u8>(*text++);
					return static_cast<char32_t>(((first & 0x0Fu) << 12) | ((second & 0x3Fu) << 6) | (third & 0x3Fu));
				}

				if ((first & 0xF8u) == 0xF0u)
				{
					const u8 second = static_cast<u8>(*text++);
					const u8 third  = static_cast<u8>(*text++);
					const u8 fourth = static_cast<u8>(*text++);
					return static_cast<char32_t>(((first & 0x07u) << 18) | ((second & 0x3Fu) << 12) | ((third & 0x3Fu) << 6) |
					                             (fourth & 0x3Fu));
				}

				return U'\0';
			}

			static EventHeader make_header(EventTypeId type_id, EventFlags flags, Uint32 window_id = 0, Identifier source_id = 0)
			{
				EventHeader header = Trinex::EventSystem::instance()->make_header(type_id, flags);
				header.timestamp   = SDL_GetTicks64();
				header.window_id   = static_cast<Identifier>(window_id);
				header.source_id   = source_id;
				return header;
			}

			static void submit_window_event(Uint32 window_id, WindowEventKind kind, i32 x = 0, i32 y = 0)
			{
				RawInputEvent event;
				event.type            = RawInputEventType::Window;
				event.device_type     = InputDeviceType::Virtual;
				event.header          = make_header(EventTypeIds::Window, EventFlags::WindowEvent, window_id);
				event.window.header   = event.header;
				event.window.kind     = kind;
				event.window.position = {x, y};
				Trinex::EventSystem::instance()->submit_raw_event(event);
			}

			static void process_window_event()
			{
				const Uint32 window_id = m_event.window.windowID;
				const i32 x            = m_event.window.data1;
				const i32 y            = m_event.window.data2;

				switch (m_event.window.event)
				{
					case SDL_WINDOWEVENT_SHOWN: submit_window_event(window_id, WindowEventKind::Shown); break;
					case SDL_WINDOWEVENT_HIDDEN: submit_window_event(window_id, WindowEventKind::Hidden); break;

					case SDL_WINDOWEVENT_MOVED:
					{
						if (Window* window = Trinex::WindowManager::instance()->find(window_id))
						{
							auto info = Platform::monitor_info(window->monitor_index());
							submit_window_event(window_id, WindowEventKind::Moved, x,
							                    static_cast<i32>(info.size.y) - (y + static_cast<i32>(window->size().y)));
						}
						else
						{
							submit_window_event(window_id, WindowEventKind::Moved, x, y);
						}
						break;
					}

					case SDL_WINDOWEVENT_RESIZED:
					case SDL_WINDOWEVENT_SIZE_CHANGED:
					{
						RawInputEvent event;
						event.type          = RawInputEventType::Window;
						event.device_type   = InputDeviceType::Virtual;
						event.header        = make_header(EventTypeIds::Window, EventFlags::WindowEvent, window_id);
						event.window.header = event.header;
						event.window.kind   = WindowEventKind::Resized;
						event.window.size   = {static_cast<u32>(x), static_cast<u32>(y)};

						if (WindowSDL* window = reinterpret_cast<WindowSDL*>(Trinex::WindowManager::instance()->find(window_id)))
						{
							window->m_size.store({x, y});
						}

						Trinex::EventSystem::instance()->submit_raw_event(event);
						break;
					}

					case SDL_WINDOWEVENT_TAKE_FOCUS:
					case SDL_WINDOWEVENT_FOCUS_GAINED: submit_window_event(window_id, WindowEventKind::FocusGained); break;

					case SDL_WINDOWEVENT_FOCUS_LOST: submit_window_event(window_id, WindowEventKind::FocusLost); break;
					case SDL_WINDOWEVENT_CLOSE: submit_window_event(window_id, WindowEventKind::CloseRequested); break;

					default: break;
				}
			}

			static void process_event()
			{
				switch (m_event.type)
				{
					case SDL_QUIT:
					{
						RawInputEvent event;
						event.type          = RawInputEventType::Window;
						event.device_type   = InputDeviceType::Virtual;
						event.header        = make_header(EventTypeIds::Quit, EventFlags::WindowEvent);
						event.window.header = event.header;
						event.window.kind   = WindowEventKind::CloseRequested;
						Trinex::EventSystem::instance()->submit_raw_event(event);
						break;
					}

					case SDL_WINDOWEVENT: process_window_event(); break;

					case SDL_KEYDOWN:
					case SDL_KEYUP:
					{
						const Uint32 window_id = m_event.key.windowID;
						const DeviceId device  = make_window_device_id(window_id, InputDeviceType::Keyboard);
						const bool is_pressed  = m_event.type == SDL_KEYDOWN;
						const bool is_repeat   = m_event.key.repeat != 0;
						RawInputEvent event;
						event.type          = RawInputEventType::Key;
						event.device_id     = device;
						event.user_id       = 0;
						event.device_type   = InputDeviceType::Keyboard;
						event.header        = make_header(EventTypeIds::Key, EventFlags::KeyboardEvent, window_id, device);
						event.key.header    = event.header;
						event.key.kind      = is_pressed ? (is_repeat ? KeyEventKind::Repeated : KeyEventKind::Pressed)
						                                 : KeyEventKind::Released;
						event.key.is_repeat = is_repeat;
						event.key.scan_code = static_cast<u32>(m_event.key.keysym.scancode);
						event.key.key_code  = static_cast<u32>(m_event.key.keysym.scancode);
						Trinex::EventSystem::instance()->submit_raw_event(event);
						break;
					}

					case SDL_MOUSEMOTION:
					{
						const Uint32 window_id = m_event.motion.windowID;
						const DeviceId device  = make_window_device_id(window_id, InputDeviceType::Mouse);
						const i32 height       = window_height(window_id);
						RawInputEvent event;
						event.type           = RawInputEventType::Pointer;
						event.device_id      = device;
						event.user_id        = 0;
						event.device_type    = InputDeviceType::Mouse;
						event.header         = make_header(EventTypeIds::Pointer, EventFlags::PointerEvent, window_id, device);
						event.pointer.header = event.header;
						event.pointer.kind   = PointerEventKind::Moved;
						event.pointer.pointer_id      = 0;
						event.pointer.screen_position = {static_cast<f32>(m_event.motion.x),
						                                 static_cast<f32>(height - m_event.motion.y)};
						event.pointer.delta = {static_cast<f32>(m_event.motion.xrel), static_cast<f32>(-m_event.motion.yrel)};
						Trinex::EventSystem::instance()->submit_raw_event(event);
						break;
					}

					case SDL_MOUSEBUTTONDOWN:
					case SDL_MOUSEBUTTONUP:
					{
						const Uint32 window_id = m_event.button.windowID;
						const DeviceId device  = make_window_device_id(window_id, InputDeviceType::Mouse);
						const i32 height       = window_height(window_id);
						RawInputEvent event;
						event.type           = RawInputEventType::Pointer;
						event.device_id      = device;
						event.user_id        = 0;
						event.device_type    = InputDeviceType::Mouse;
						event.header         = make_header(EventTypeIds::Pointer, EventFlags::PointerEvent, window_id, device);
						event.pointer.header = event.header;
						event.pointer.kind   = m_event.type == SDL_MOUSEBUTTONDOWN ? PointerEventKind::ButtonPressed
						                                                           : PointerEventKind::ButtonReleased;
						event.pointer.pointer_id      = 0;
						event.pointer.button          = static_cast<u32>(map_mouse_button(m_event.button.button));
						event.pointer.screen_position = {static_cast<f32>(m_event.button.x),
						                                 static_cast<f32>(height - m_event.button.y)};
						Trinex::EventSystem::instance()->submit_raw_event(event);
						break;
					}

					case SDL_MOUSEWHEEL:
					{
						const Uint32 window_id = m_event.wheel.windowID;
						const DeviceId device  = make_window_device_id(window_id, InputDeviceType::Mouse);
						RawInputEvent event;
						event.type           = RawInputEventType::Pointer;
						event.device_id      = device;
						event.user_id        = 0;
						event.device_type    = InputDeviceType::Mouse;
						event.header         = make_header(EventTypeIds::Pointer, EventFlags::PointerEvent, window_id, device);
						event.pointer.header = event.header;
						event.pointer.kind   = PointerEventKind::Wheel;
						event.pointer.pointer_id  = 0;
						event.pointer.wheel_delta = {m_event.wheel.preciseX, m_event.wheel.preciseY};
						Trinex::EventSystem::instance()->submit_raw_event(event);
						break;
					}

					case SDL_TEXTINPUT:
					{
						const Uint32 window_id = m_event.text.windowID;
						const DeviceId device  = make_window_device_id(window_id, InputDeviceType::Keyboard);
						const char* text       = m_event.text.text;

						while (*text)
						{
							RawInputEvent event;
							event.type        = RawInputEventType::TextInput;
							event.device_id   = device;
							event.user_id     = 0;
							event.device_type = InputDeviceType::Keyboard;
							event.header = make_header(EventTypeIds::TextInput, EventFlags::TextInputEvent, window_id, device);
							event.text_input.header       = event.header;
							event.text_input.codepoint    = decode_utf8_codepoint(text);
							event.text_input.is_composing = false;
							Trinex::EventSystem::instance()->submit_raw_event(event);
						}
						break;
					}

					case SDL_CONTROLLERDEVICEADDED:
					{
						if (SDL_GameController* controller = SDL_GameControllerOpen(m_event.cdevice.which))
						{
							SDL_Joystick* joystick          = SDL_GameControllerGetJoystick(controller);
							SDL_JoystickID joystick_id      = SDL_JoystickInstanceID(joystick);
							m_game_controllers[joystick_id] = controller;

							RawInputEvent event;
							event.type        = RawInputEventType::DeviceChange;
							event.device_id   = make_gamepad_device_id(joystick_id);
							event.user_id     = 0;
							event.device_type = InputDeviceType::Gamepad;
							event.header = make_header(EventTypeIds::DeviceChange, EventFlags::Undefined, 0, event.device_id);
							event.device_change.header      = event.header;
							event.device_change.kind        = DeviceChangeKind::Added;
							event.device_change.device_id   = event.device_id;
							event.device_change.device_type = event.device_type;
							event.device_change.user_id     = event.user_id;
							Trinex::EventSystem::instance()->submit_raw_event(event);
						}
						break;
					}

					case SDL_CONTROLLERDEVICEREMOVED:
					{
						if (auto controller = m_game_controllers.find(m_event.cdevice.which);
						    controller != m_game_controllers.end())
						{
							RawInputEvent event;
							event.type        = RawInputEventType::DeviceChange;
							event.device_id   = make_gamepad_device_id(m_event.cdevice.which);
							event.user_id     = 0;
							event.device_type = InputDeviceType::Gamepad;
							event.header = make_header(EventTypeIds::DeviceChange, EventFlags::Undefined, 0, event.device_id);
							event.device_change.header      = event.header;
							event.device_change.kind        = DeviceChangeKind::Removed;
							event.device_change.device_id   = event.device_id;
							event.device_change.device_type = event.device_type;
							event.device_change.user_id     = event.user_id;
							Trinex::EventSystem::instance()->submit_raw_event(event);

							SDL_GameControllerClose(controller->second);
							m_game_controllers.erase(m_event.cdevice.which);
						}
						break;
					}

					case SDL_CONTROLLERDEVICEREMAPPED:
					{
						RawInputEvent event;
						event.type        = RawInputEventType::DeviceChange;
						event.device_id   = make_gamepad_device_id(m_event.cdevice.which);
						event.user_id     = 0;
						event.device_type = InputDeviceType::Gamepad;
						event.header      = make_header(EventTypeIds::DeviceChange, EventFlags::Undefined, 0, event.device_id);
						event.device_change.header      = event.header;
						event.device_change.kind        = DeviceChangeKind::Remapped;
						event.device_change.device_id   = event.device_id;
						event.device_change.device_type = event.device_type;
						event.device_change.user_id     = event.user_id;
						Trinex::EventSystem::instance()->submit_raw_event(event);
						break;
					}

					case SDL_CONTROLLERAXISMOTION:
					{
						if (m_game_controllers.find(m_event.caxis.which) != m_game_controllers.end())
						{
							RawInputEvent event;
							event.type           = RawInputEventType::Gamepad;
							event.device_id      = make_gamepad_device_id(m_event.caxis.which);
							event.user_id        = 0;
							event.device_type    = InputDeviceType::Gamepad;
							event.header         = make_header(EventTypeIds::Gamepad, EventFlags::Undefined, 0, event.device_id);
							event.gamepad.header = event.header;
							event.gamepad.kind   = GamepadEventKind::AxisMotion;
							event.gamepad.axis   = map_gamepad_axis(m_event.caxis.axis);
							event.gamepad.value =
							        static_cast<f32>(m_event.caxis.value) / static_cast<f32>(std::numeric_limits<i16>::max());
							Trinex::EventSystem::instance()->submit_raw_event(event);
						}
						break;
					}

					case SDL_CONTROLLERBUTTONDOWN:
					case SDL_CONTROLLERBUTTONUP:
					{
						if (m_game_controllers.find(m_event.cbutton.which) != m_game_controllers.end())
						{
							RawInputEvent event;
							event.type           = RawInputEventType::Gamepad;
							event.device_id      = make_gamepad_device_id(m_event.cbutton.which);
							event.user_id        = 0;
							event.device_type    = InputDeviceType::Gamepad;
							event.header         = make_header(EventTypeIds::Gamepad, EventFlags::Undefined, 0, event.device_id);
							event.gamepad.header = event.header;
							event.gamepad.kind   = m_event.type == SDL_CONTROLLERBUTTONDOWN ? GamepadEventKind::ButtonPressed
							                                                                : GamepadEventKind::ButtonReleased;
							event.gamepad.button = map_gamepad_button(m_event.cbutton.button);
							Trinex::EventSystem::instance()->submit_raw_event(event);
						}
						break;
					}

					default: break;
				}
			}

			static void pool_events_loop()
			{
				while (SDL_PollEvent(&m_event))
				{
					process_event();
				}
			}
		}// namespace

		ENGINE_EXPORT void pool_events()
		{
			pool_events_loop();
		}

		ENGINE_EXPORT void wait_for_events()
		{
			SDL_WaitEvent(&m_event);
			process_event();
			pool_events_loop();
		}
	}// namespace EventSystem
}// namespace Trinex::Platform
