#include <Core/base_engine.hpp>
#include <Core/event.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/threading.hpp>
#include <Platform/platform.hpp>
#include <Systems/Migration/event_system.hpp>
#include <Systems/Migration/input_system.hpp>
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
			static Migration::DeviceId make_window_device_id(Uint32 window_id, Migration::InputDeviceType type)
			{
				return (static_cast<Migration::DeviceId>(type) << 56) | static_cast<Migration::DeviceId>(window_id);
			}

			static Migration::DeviceId make_gamepad_device_id(SDL_JoystickID joystick_id)
			{
				return (static_cast<Migration::DeviceId>(Migration::InputDeviceType::Gamepad) << 56) |
				       static_cast<Migration::DeviceId>(static_cast<u32>(joystick_id));
			}

			static Migration::MouseButton map_mouse_button(Uint8 button)
			{
				switch (button)
				{
					case SDL_BUTTON_LEFT: return Migration::MouseButton::Left;
					case SDL_BUTTON_RIGHT: return Migration::MouseButton::Right;
					case SDL_BUTTON_MIDDLE: return Migration::MouseButton::Middle;
					case SDL_BUTTON_X1: return Migration::MouseButton::X1;
					case SDL_BUTTON_X2: return Migration::MouseButton::X2;
					default: return Migration::MouseButton::None;
				}
			}

			static Migration::GamepadAxis map_gamepad_axis(Uint8 axis)
			{
				switch (axis)
				{
					case SDL_CONTROLLER_AXIS_LEFTX: return Migration::GamepadAxis::LeftX;
					case SDL_CONTROLLER_AXIS_LEFTY: return Migration::GamepadAxis::LeftY;
					case SDL_CONTROLLER_AXIS_RIGHTX: return Migration::GamepadAxis::RightX;
					case SDL_CONTROLLER_AXIS_RIGHTY: return Migration::GamepadAxis::RightY;
					case SDL_CONTROLLER_AXIS_TRIGGERLEFT: return Migration::GamepadAxis::LeftTrigger;
					case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: return Migration::GamepadAxis::RightTrigger;
					default: return Migration::GamepadAxis::None;
				}
			}

			static Migration::GamepadButton map_gamepad_button(Uint8 button)
			{
				switch (button)
				{
					case SDL_CONTROLLER_BUTTON_A: return Migration::GamepadButton::FaceBottom;
					case SDL_CONTROLLER_BUTTON_B: return Migration::GamepadButton::FaceRight;
					case SDL_CONTROLLER_BUTTON_X: return Migration::GamepadButton::FaceLeft;
					case SDL_CONTROLLER_BUTTON_Y: return Migration::GamepadButton::FaceTop;
					case SDL_CONTROLLER_BUTTON_BACK: return Migration::GamepadButton::Select;
					case SDL_CONTROLLER_BUTTON_GUIDE: return Migration::GamepadButton::Guide;
					case SDL_CONTROLLER_BUTTON_START: return Migration::GamepadButton::Start;
					case SDL_CONTROLLER_BUTTON_LEFTSTICK: return Migration::GamepadButton::LeftStick;
					case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return Migration::GamepadButton::RightStick;
					case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return Migration::GamepadButton::LeftShoulder;
					case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return Migration::GamepadButton::RightShoulder;
					case SDL_CONTROLLER_BUTTON_DPAD_UP: return Migration::GamepadButton::DPadUp;
					case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return Migration::GamepadButton::DPadDown;
					case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return Migration::GamepadButton::DPadLeft;
					case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return Migration::GamepadButton::DPadRight;
#ifdef SDL_CONTROLLER_BUTTON_MISC1
					case SDL_CONTROLLER_BUTTON_MISC1: return Migration::GamepadButton::Misc1;
#endif
#ifdef SDL_CONTROLLER_BUTTON_PADDLE1
					case SDL_CONTROLLER_BUTTON_PADDLE1: return Migration::GamepadButton::Paddle1;
					case SDL_CONTROLLER_BUTTON_PADDLE2: return Migration::GamepadButton::Paddle2;
					case SDL_CONTROLLER_BUTTON_PADDLE3: return Migration::GamepadButton::Paddle3;
					case SDL_CONTROLLER_BUTTON_PADDLE4: return Migration::GamepadButton::Paddle4;
#endif
#ifdef SDL_CONTROLLER_BUTTON_TOUCHPAD
					case SDL_CONTROLLER_BUTTON_TOUCHPAD: return Migration::GamepadButton::Touchpad;
#endif
					default: return Migration::GamepadButton::None;
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

			static Migration::EventHeader make_header(Migration::EventTypeId type_id, Migration::EventFlags flags,
			                                          Uint32 window_id = 0, Identifier source_id = 0)
			{
				Migration::EventHeader header = Migration::EventSystem::instance()->make_header(type_id, flags);
				header.timestamp              = SDL_GetTicks64();
				header.window_id              = static_cast<Identifier>(window_id);
				header.source_id              = source_id;
				return header;
			}

			static void submit_window_event(Uint32 window_id, Migration::WindowEventKind kind, i32 x = 0, i32 y = 0)
			{
				Migration::RawInputEvent event;
				event.type          = Migration::RawInputEventType::Window;
				event.device_type   = Migration::InputDeviceType::Virtual;
				event.header        = make_header(Migration::EventTypeIds::Window, Migration::EventFlags::WindowEvent, window_id);
				event.window.header = event.header;
				event.window.kind   = kind;
				event.window.position = {x, y};
				Migration::EventSystem::instance()->submit_raw_event(event);
			}

			static void process_window_event()
			{
				const Uint32 window_id = m_event.window.windowID;
				const i32 x            = m_event.window.data1;
				const i32 y            = m_event.window.data2;

				switch (m_event.window.event)
				{
					case SDL_WINDOWEVENT_SHOWN: submit_window_event(window_id, Migration::WindowEventKind::Shown); break;
					case SDL_WINDOWEVENT_HIDDEN: submit_window_event(window_id, Migration::WindowEventKind::Hidden); break;

					case SDL_WINDOWEVENT_MOVED:
					{
						if (Window* window = Trinex::WindowManager::instance()->find(window_id))
						{
							auto info = Platform::monitor_info(window->monitor_index());
							submit_window_event(window_id, Migration::WindowEventKind::Moved, x,
							                    static_cast<i32>(info.size.y) - (y + static_cast<i32>(window->size().y)));
						}
						else
						{
							submit_window_event(window_id, Migration::WindowEventKind::Moved, x, y);
						}
						break;
					}

					case SDL_WINDOWEVENT_RESIZED:
					case SDL_WINDOWEVENT_SIZE_CHANGED:
					{
						Migration::RawInputEvent event;
						event.type        = Migration::RawInputEventType::Window;
						event.device_type = Migration::InputDeviceType::Virtual;
						event.header =
						        make_header(Migration::EventTypeIds::Window, Migration::EventFlags::WindowEvent, window_id);
						event.window.header = event.header;
						event.window.kind   = Migration::WindowEventKind::Resized;
						event.window.size   = {static_cast<u32>(x), static_cast<u32>(y)};

						if (WindowSDL* window = reinterpret_cast<WindowSDL*>(Trinex::WindowManager::instance()->find(window_id)))
						{
							window->m_size.store({x, y});
						}

						Migration::EventSystem::instance()->submit_raw_event(event);
						break;
					}

					case SDL_WINDOWEVENT_TAKE_FOCUS:
					case SDL_WINDOWEVENT_FOCUS_GAINED:
						submit_window_event(window_id, Migration::WindowEventKind::FocusGained);
						break;

					case SDL_WINDOWEVENT_FOCUS_LOST: submit_window_event(window_id, Migration::WindowEventKind::FocusLost); break;
					case SDL_WINDOWEVENT_CLOSE: submit_window_event(window_id, Migration::WindowEventKind::CloseRequested); break;

					default: break;
				}
			}

			static void process_event()
			{
				switch (m_event.type)
				{
					case SDL_QUIT:
					{
						Migration::RawInputEvent event;
						event.type          = Migration::RawInputEventType::Window;
						event.device_type   = Migration::InputDeviceType::Virtual;
						event.header        = make_header(Migration::EventTypeIds::Quit, Migration::EventFlags::WindowEvent);
						event.window.header = event.header;
						event.window.kind   = Migration::WindowEventKind::CloseRequested;
						Migration::EventSystem::instance()->submit_raw_event(event);
						break;
					}

					case SDL_WINDOWEVENT: process_window_event(); break;

					case SDL_KEYDOWN:
					case SDL_KEYUP:
					{
						const Uint32 window_id           = m_event.key.windowID;
						const Migration::DeviceId device = make_window_device_id(window_id, Migration::InputDeviceType::Keyboard);
						const bool is_pressed            = m_event.type == SDL_KEYDOWN;
						const bool is_repeat             = m_event.key.repeat != 0;
						Migration::RawInputEvent event;
						event.type        = Migration::RawInputEventType::Key;
						event.device_id   = device;
						event.user_id     = 0;
						event.device_type = Migration::InputDeviceType::Keyboard;
						event.header = make_header(Migration::EventTypeIds::Key, Migration::EventFlags::KeyboardEvent, window_id,
						                           device);
						event.key.header = event.header;
						event.key.kind =
						        is_pressed ? (is_repeat ? Migration::KeyEventKind::Repeated : Migration::KeyEventKind::Pressed)
						                   : Migration::KeyEventKind::Released;
						event.key.is_repeat = is_repeat;
						event.key.scan_code = static_cast<u32>(m_event.key.keysym.scancode);
						event.key.key_code  = static_cast<u32>(m_event.key.keysym.scancode);
						Migration::EventSystem::instance()->submit_raw_event(event);
						break;
					}

					case SDL_MOUSEMOTION:
					{
						const Uint32 window_id           = m_event.motion.windowID;
						const Migration::DeviceId device = make_window_device_id(window_id, Migration::InputDeviceType::Mouse);
						const i32 height                 = window_height(window_id);
						Migration::RawInputEvent event;
						event.type           = Migration::RawInputEventType::Pointer;
						event.device_id      = device;
						event.user_id        = 0;
						event.device_type    = Migration::InputDeviceType::Mouse;
						event.header         = make_header(Migration::EventTypeIds::Pointer, Migration::EventFlags::PointerEvent,
						                                   window_id, device);
						event.pointer.header = event.header;
						event.pointer.kind   = Migration::PointerEventKind::Moved;
						event.pointer.pointer_id      = 0;
						event.pointer.screen_position = {static_cast<f32>(m_event.motion.x),
						                                 static_cast<f32>(height - m_event.motion.y)};
						event.pointer.delta = {static_cast<f32>(m_event.motion.xrel), static_cast<f32>(-m_event.motion.yrel)};
						Migration::EventSystem::instance()->submit_raw_event(event);
						break;
					}

					case SDL_MOUSEBUTTONDOWN:
					case SDL_MOUSEBUTTONUP:
					{
						const Uint32 window_id           = m_event.button.windowID;
						const Migration::DeviceId device = make_window_device_id(window_id, Migration::InputDeviceType::Mouse);
						const i32 height                 = window_height(window_id);
						Migration::RawInputEvent event;
						event.type           = Migration::RawInputEventType::Pointer;
						event.device_id      = device;
						event.user_id        = 0;
						event.device_type    = Migration::InputDeviceType::Mouse;
						event.header         = make_header(Migration::EventTypeIds::Pointer, Migration::EventFlags::PointerEvent,
						                                   window_id, device);
						event.pointer.header = event.header;
						event.pointer.kind   = m_event.type == SDL_MOUSEBUTTONDOWN ? Migration::PointerEventKind::ButtonPressed
						                                                           : Migration::PointerEventKind::ButtonReleased;
						event.pointer.pointer_id      = 0;
						event.pointer.button          = static_cast<u32>(map_mouse_button(m_event.button.button));
						event.pointer.screen_position = {static_cast<f32>(m_event.button.x),
						                                 static_cast<f32>(height - m_event.button.y)};
						Migration::EventSystem::instance()->submit_raw_event(event);
						break;
					}

					case SDL_MOUSEWHEEL:
					{
						const Uint32 window_id           = m_event.wheel.windowID;
						const Migration::DeviceId device = make_window_device_id(window_id, Migration::InputDeviceType::Mouse);
						Migration::RawInputEvent event;
						event.type           = Migration::RawInputEventType::Pointer;
						event.device_id      = device;
						event.user_id        = 0;
						event.device_type    = Migration::InputDeviceType::Mouse;
						event.header         = make_header(Migration::EventTypeIds::Pointer, Migration::EventFlags::PointerEvent,
						                                   window_id, device);
						event.pointer.header = event.header;
						event.pointer.kind   = Migration::PointerEventKind::Wheel;
						event.pointer.pointer_id  = 0;
						event.pointer.wheel_delta = {m_event.wheel.preciseX, m_event.wheel.preciseY};
						Migration::EventSystem::instance()->submit_raw_event(event);
						break;
					}

					case SDL_TEXTINPUT:
					{
						const Uint32 window_id           = m_event.text.windowID;
						const Migration::DeviceId device = make_window_device_id(window_id, Migration::InputDeviceType::Keyboard);
						const char* text                 = m_event.text.text;

						while (*text)
						{
							Migration::RawInputEvent event;
							event.type        = Migration::RawInputEventType::TextInput;
							event.device_id   = device;
							event.user_id     = 0;
							event.device_type = Migration::InputDeviceType::Keyboard;
							event.header = make_header(Migration::EventTypeIds::TextInput, Migration::EventFlags::TextInputEvent,
							                           window_id, device);
							event.text_input.header       = event.header;
							event.text_input.codepoint    = decode_utf8_codepoint(text);
							event.text_input.is_composing = false;
							Migration::EventSystem::instance()->submit_raw_event(event);
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

							Migration::RawInputEvent event;
							event.type        = Migration::RawInputEventType::DeviceChange;
							event.device_id   = make_gamepad_device_id(joystick_id);
							event.user_id     = 0;
							event.device_type = Migration::InputDeviceType::Gamepad;
							event.header      = make_header(Migration::EventTypeIds::DeviceChange, Migration::EventFlags::None, 0,
							                                event.device_id);
							event.device_change.header      = event.header;
							event.device_change.kind        = Migration::DeviceChangeKind::Added;
							event.device_change.device_id   = event.device_id;
							event.device_change.device_type = event.device_type;
							event.device_change.user_id     = event.user_id;
							Migration::EventSystem::instance()->submit_raw_event(event);
						}
						break;
					}

					case SDL_CONTROLLERDEVICEREMOVED:
					{
						if (auto controller = m_game_controllers.find(m_event.cdevice.which);
						    controller != m_game_controllers.end())
						{
							Migration::RawInputEvent event;
							event.type        = Migration::RawInputEventType::DeviceChange;
							event.device_id   = make_gamepad_device_id(m_event.cdevice.which);
							event.user_id     = 0;
							event.device_type = Migration::InputDeviceType::Gamepad;
							event.header      = make_header(Migration::EventTypeIds::DeviceChange, Migration::EventFlags::None, 0,
							                                event.device_id);
							event.device_change.header      = event.header;
							event.device_change.kind        = Migration::DeviceChangeKind::Removed;
							event.device_change.device_id   = event.device_id;
							event.device_change.device_type = event.device_type;
							event.device_change.user_id     = event.user_id;
							Migration::EventSystem::instance()->submit_raw_event(event);

							SDL_GameControllerClose(controller->second);
							m_game_controllers.erase(m_event.cdevice.which);
						}
						break;
					}

					case SDL_CONTROLLERDEVICEREMAPPED:
					{
						Migration::RawInputEvent event;
						event.type        = Migration::RawInputEventType::DeviceChange;
						event.device_id   = make_gamepad_device_id(m_event.cdevice.which);
						event.user_id     = 0;
						event.device_type = Migration::InputDeviceType::Gamepad;
						event.header      = make_header(Migration::EventTypeIds::DeviceChange, Migration::EventFlags::None, 0,
						                                event.device_id);
						event.device_change.header      = event.header;
						event.device_change.kind        = Migration::DeviceChangeKind::Remapped;
						event.device_change.device_id   = event.device_id;
						event.device_change.device_type = event.device_type;
						event.device_change.user_id     = event.user_id;
						Migration::EventSystem::instance()->submit_raw_event(event);
						break;
					}

					case SDL_CONTROLLERAXISMOTION:
					{
						if (m_game_controllers.find(m_event.caxis.which) != m_game_controllers.end())
						{
							Migration::RawInputEvent event;
							event.type           = Migration::RawInputEventType::Gamepad;
							event.device_id      = make_gamepad_device_id(m_event.caxis.which);
							event.user_id        = 0;
							event.device_type    = Migration::InputDeviceType::Gamepad;
							event.header         = make_header(Migration::EventTypeIds::Gamepad, Migration::EventFlags::None, 0,
							                                   event.device_id);
							event.gamepad.header = event.header;
							event.gamepad.kind   = Migration::GamepadEventKind::AxisMotion;
							event.gamepad.axis   = map_gamepad_axis(m_event.caxis.axis);
							event.gamepad.value =
							        static_cast<f32>(m_event.caxis.value) / static_cast<f32>(std::numeric_limits<i16>::max());
							Migration::EventSystem::instance()->submit_raw_event(event);
						}
						break;
					}

					case SDL_CONTROLLERBUTTONDOWN:
					case SDL_CONTROLLERBUTTONUP:
					{
						if (m_game_controllers.find(m_event.cbutton.which) != m_game_controllers.end())
						{
							Migration::RawInputEvent event;
							event.type           = Migration::RawInputEventType::Gamepad;
							event.device_id      = make_gamepad_device_id(m_event.cbutton.which);
							event.user_id        = 0;
							event.device_type    = Migration::InputDeviceType::Gamepad;
							event.header         = make_header(Migration::EventTypeIds::Gamepad, Migration::EventFlags::None, 0,
							                                   event.device_id);
							event.gamepad.header = event.header;
							event.gamepad.kind   = m_event.type == SDL_CONTROLLERBUTTONDOWN
							                               ? Migration::GamepadEventKind::ButtonPressed
							                               : Migration::GamepadEventKind::ButtonReleased;
							event.gamepad.button = map_gamepad_button(m_event.cbutton.button);
							Migration::EventSystem::instance()->submit_raw_event(event);
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
