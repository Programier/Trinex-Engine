#include <Core/base_engine.hpp>
#include <Core/event.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Platform/platform.hpp>
#include <Systems/event_system.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <sdl_window.hpp>

namespace Engine::Platform
{
	ENGINE_EXPORT size_t monitors_count()
	{
		return static_cast<size_t>(SDL_GetNumVideoDisplays());
	}

	ENGINE_EXPORT MonitorInfo monitor_info(Index monitor_index)
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
}// namespace Engine::Platform


namespace Engine::Platform::WindowManager
{
	static Map<Sint32, SDL_GameController*> m_game_controllers;

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
}// namespace Engine::Platform::WindowManager

namespace Engine::Platform::EventSystem
{
	template<typename Type>
	using ValueMap = const Map<Uint8, Type>;

	static ValueMap<GameController::Axis> axis_type = {
			{SDL_CONTROLLER_AXIS_INVALID, GameController::Axis::Unknown},
	        {SDL_CONTROLLER_AXIS_LEFTX, GameController::Axis::LeftX},
	        {SDL_CONTROLLER_AXIS_LEFTY, GameController::Axis::LeftY},
	        {SDL_CONTROLLER_AXIS_TRIGGERLEFT, GameController::Axis::TriggerLeft},
	        {SDL_CONTROLLER_AXIS_RIGHTX, GameController::Axis::RightX},
	        {SDL_CONTROLLER_AXIS_RIGHTY, GameController::Axis::RightY},
	        {SDL_CONTROLLER_AXIS_TRIGGERLEFT, GameController::Axis::TriggerRight},
	};

	static ValueMap<Mouse::Button> mouse_buttons = {{SDL_BUTTON_LEFT, Mouse::Button::Left},
	                                                {SDL_BUTTON_MIDDLE, Mouse::Button::Middle},
	                                                {SDL_BUTTON_RIGHT, Mouse::Button::Right},
	                                                {SDL_BUTTON_X1, Mouse::Button::Back},
	                                                {SDL_BUTTON_X2, Mouse::Button::Forward}};

	static ValueMap<Keyboard::Key> keys = {
	        {SDL_SCANCODE_UNKNOWN, Keyboard::Key::Unknown},
	        {SDL_SCANCODE_SPACE, Keyboard::Key::Space},
	        {SDL_SCANCODE_APOSTROPHE, Keyboard::Key::Apostrophe},
	        {SDL_SCANCODE_COMMA, Keyboard::Key::Comma},
	        {SDL_SCANCODE_MINUS, Keyboard::Key::Minus},
	        {SDL_SCANCODE_PERIOD, Keyboard::Key::Period},
	        {SDL_SCANCODE_SLASH, Keyboard::Key::Slash},
	        {SDL_SCANCODE_0, Keyboard::Key::Num0},
	        {SDL_SCANCODE_1, Keyboard::Key::Num1},
	        {SDL_SCANCODE_2, Keyboard::Key::Num2},
	        {SDL_SCANCODE_3, Keyboard::Key::Num3},
	        {SDL_SCANCODE_4, Keyboard::Key::Num4},
	        {SDL_SCANCODE_5, Keyboard::Key::Num5},
	        {SDL_SCANCODE_6, Keyboard::Key::Num6},
	        {SDL_SCANCODE_7, Keyboard::Key::Num7},
	        {SDL_SCANCODE_8, Keyboard::Key::Num8},
	        {SDL_SCANCODE_9, Keyboard::Key::Num9},
	        {SDL_SCANCODE_SEMICOLON, Keyboard::Key::Semicolon},
	        {SDL_SCANCODE_EQUALS, Keyboard::Key::Equal},
	        {SDL_SCANCODE_A, Keyboard::Key::A},
	        {SDL_SCANCODE_B, Keyboard::Key::B},
	        {SDL_SCANCODE_C, Keyboard::Key::C},
	        {SDL_SCANCODE_D, Keyboard::Key::D},
	        {SDL_SCANCODE_E, Keyboard::Key::E},
	        {SDL_SCANCODE_F, Keyboard::Key::F},
	        {SDL_SCANCODE_G, Keyboard::Key::G},
	        {SDL_SCANCODE_H, Keyboard::Key::H},
	        {SDL_SCANCODE_I, Keyboard::Key::I},
	        {SDL_SCANCODE_J, Keyboard::Key::J},
	        {SDL_SCANCODE_K, Keyboard::Key::K},
	        {SDL_SCANCODE_L, Keyboard::Key::L},
	        {SDL_SCANCODE_M, Keyboard::Key::M},
	        {SDL_SCANCODE_N, Keyboard::Key::N},
	        {SDL_SCANCODE_O, Keyboard::Key::O},
	        {SDL_SCANCODE_P, Keyboard::Key::P},
	        {SDL_SCANCODE_Q, Keyboard::Key::Q},
	        {SDL_SCANCODE_R, Keyboard::Key::R},
	        {SDL_SCANCODE_S, Keyboard::Key::S},
	        {SDL_SCANCODE_T, Keyboard::Key::T},
	        {SDL_SCANCODE_U, Keyboard::Key::U},
	        {SDL_SCANCODE_V, Keyboard::Key::V},
	        {SDL_SCANCODE_W, Keyboard::Key::W},
	        {SDL_SCANCODE_X, Keyboard::Key::X},
	        {SDL_SCANCODE_Y, Keyboard::Key::Y},
	        {SDL_SCANCODE_Z, Keyboard::Key::Z},
	        {SDL_SCANCODE_LEFTBRACKET, Keyboard::Key::LeftBracket},
	        {SDL_SCANCODE_BACKSLASH, Keyboard::Key::Backslash},
	        {SDL_SCANCODE_RIGHTBRACKET, Keyboard::Key::RightBracket},
	        {SDL_SCANCODE_GRAVE, Keyboard::Key::GraveAccent},
	        {SDL_SCANCODE_WWW, Keyboard::Key::Explorer},
	        {SDL_SCANCODE_ESCAPE, Keyboard::Key::Escape},
	        {SDL_SCANCODE_RETURN, Keyboard::Key::Enter},
	        {SDL_SCANCODE_TAB, Keyboard::Key::Tab},
	        {SDL_SCANCODE_BACKSPACE, Keyboard::Key::Backspace},
	        {SDL_SCANCODE_INSERT, Keyboard::Key::Insert},
	        {SDL_SCANCODE_DELETE, Keyboard::Key::Delete},
	        {SDL_SCANCODE_RIGHT, Keyboard::Key::Right},
	        {SDL_SCANCODE_LEFT, Keyboard::Key::Left},
	        {SDL_SCANCODE_DOWN, Keyboard::Key::Down},
	        {SDL_SCANCODE_UP, Keyboard::Key::Up},
	        {SDL_SCANCODE_PAGEUP, Keyboard::Key::PageUp},
	        {SDL_SCANCODE_PAGEDOWN, Keyboard::Key::PageDown},
	        {SDL_SCANCODE_HOME, Keyboard::Key::Home},
	        {SDL_SCANCODE_END, Keyboard::Key::End},
	        {SDL_SCANCODE_CAPSLOCK, Keyboard::Key::CapsLock},
	        {SDL_SCANCODE_SCROLLLOCK, Keyboard::Key::ScrollLock},
	        {SDL_SCANCODE_NUMLOCKCLEAR, Keyboard::Key::NumLock},
	        {SDL_SCANCODE_PRINTSCREEN, Keyboard::Key::PrintScreen},
	        {SDL_SCANCODE_PAUSE, Keyboard::Key::Pause},
	        {SDL_SCANCODE_F1, Keyboard::Key::F1},
	        {SDL_SCANCODE_F2, Keyboard::Key::F2},
	        {SDL_SCANCODE_F3, Keyboard::Key::F3},
	        {SDL_SCANCODE_F4, Keyboard::Key::F4},
	        {SDL_SCANCODE_F5, Keyboard::Key::F5},
	        {SDL_SCANCODE_F6, Keyboard::Key::F6},
	        {SDL_SCANCODE_F7, Keyboard::Key::F7},
	        {SDL_SCANCODE_F8, Keyboard::Key::F8},
	        {SDL_SCANCODE_F9, Keyboard::Key::F9},
	        {SDL_SCANCODE_F10, Keyboard::Key::F10},
	        {SDL_SCANCODE_F11, Keyboard::Key::F11},
	        {SDL_SCANCODE_F12, Keyboard::Key::F12},
	        {SDL_SCANCODE_F13, Keyboard::Key::F13},
	        {SDL_SCANCODE_F14, Keyboard::Key::F14},
	        {SDL_SCANCODE_F15, Keyboard::Key::F15},
	        {SDL_SCANCODE_F16, Keyboard::Key::F16},
	        {SDL_SCANCODE_F17, Keyboard::Key::F17},
	        {SDL_SCANCODE_F18, Keyboard::Key::F18},
	        {SDL_SCANCODE_F19, Keyboard::Key::F19},
	        {SDL_SCANCODE_F20, Keyboard::Key::F20},
	        {SDL_SCANCODE_F21, Keyboard::Key::F21},
	        {SDL_SCANCODE_F22, Keyboard::Key::F22},
	        {SDL_SCANCODE_F23, Keyboard::Key::F23},
	        {SDL_SCANCODE_F24, Keyboard::Key::F24},
	        {SDL_SCANCODE_KP_0, Keyboard::Key::Kp0},
	        {SDL_SCANCODE_KP_1, Keyboard::Key::Kp1},
	        {SDL_SCANCODE_KP_2, Keyboard::Key::Kp2},
	        {SDL_SCANCODE_KP_3, Keyboard::Key::Kp3},
	        {SDL_SCANCODE_KP_4, Keyboard::Key::Kp4},
	        {SDL_SCANCODE_KP_5, Keyboard::Key::Kp5},
	        {SDL_SCANCODE_KP_6, Keyboard::Key::Kp6},
	        {SDL_SCANCODE_KP_7, Keyboard::Key::Kp7},
	        {SDL_SCANCODE_KP_8, Keyboard::Key::Kp8},
	        {SDL_SCANCODE_KP_9, Keyboard::Key::Kp9},
	        {SDL_SCANCODE_KP_DECIMAL, Keyboard::Key::KpDot},
	        {SDL_SCANCODE_KP_DIVIDE, Keyboard::Key::KpDivide},
	        {SDL_SCANCODE_KP_MULTIPLY, Keyboard::Key::KpMultiply},
	        {SDL_SCANCODE_KP_MINUS, Keyboard::Key::KpSubtract},
	        {SDL_SCANCODE_KP_PLUS, Keyboard::Key::KpAdd},
	        {SDL_SCANCODE_KP_ENTER, Keyboard::Key::KpEnter},
	        {SDL_SCANCODE_KP_EQUALS, Keyboard::Key::KpEqual},
	        {SDL_SCANCODE_LSHIFT, Keyboard::Key::LeftShift},
	        {SDL_SCANCODE_LCTRL, Keyboard::Key::LeftControl},
	        {SDL_SCANCODE_LALT, Keyboard::Key::LeftAlt},
	        {SDL_SCANCODE_LGUI, Keyboard::Key::LeftSuper},
	        {SDL_SCANCODE_RSHIFT, Keyboard::Key::RightShift},
	        {SDL_SCANCODE_RCTRL, Keyboard::Key::RightControl},
	        {SDL_SCANCODE_RALT, Keyboard::Key::RightAlt},
	        {SDL_SCANCODE_RGUI, Keyboard::Key::RightSuper},
	        {SDL_SCANCODE_MENU, Keyboard::Key::Menu},
	};

	static Event m_engine_event;
	static SDL_Event m_event;

#define new_event(t)                                                                                                             \
	m_engine_event.type = EventType::t;                                                                                          \
	callback(m_engine_event, userdata);                                                                                          \
	return

	static void process_window_event(void (*callback)(const Event& event, void* userdata), void* userdata)
	{
		int_t x = m_event.window.data1;
		int_t y = m_event.window.data2;

		switch (m_event.window.event)
		{
			case SDL_WINDOWEVENT_SHOWN:
				new_event(WindowShown);

			case SDL_WINDOWEVENT_HIDDEN:
				new_event(WindowHidden);


			case SDL_WINDOWEVENT_MOVED:
			{
				m_engine_event.window.x = x;
				Window* window          = Engine::WindowManager::instance()->find(m_event.window.windowID);
				if (window)
				{
					size_t index            = window->monitor_index();
					auto info               = Platform::monitor_info(index);
					m_engine_event.window.y = info.size.y - (y + window->size().y);
					new_event(WindowMoved);
				}
				break;
			}

			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
			{
				WindowSDL* window =
				        reinterpret_cast<WindowSDL*>(Engine::WindowManager::instance()->find(m_event.window.windowID));

				m_engine_event.window.x = x;
				m_engine_event.window.y = y;

				window->m_size.store({x, y});
				new_event(WindowResized);
			}

			case SDL_WINDOWEVENT_MINIMIZED:
				new_event(WindowMinimized);

			case SDL_WINDOWEVENT_MAXIMIZED:
				new_event(WindowMaximized);

			case SDL_WINDOWEVENT_RESTORED:
				new_event(WindowRestored);

			case SDL_WINDOWEVENT_TAKE_FOCUS:
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				new_event(WindowFocusGained);

			case SDL_WINDOWEVENT_FOCUS_LOST:
				new_event(WindowFocusLost);

			case SDL_WINDOWEVENT_CLOSE:
				new_event(WindowClose);
		}
	}

	static void process_mouse_button(void (*callback)(const Event& event, void* userdata), void* userdata)
	{
		auto& button_event = m_engine_event.mouse.button;
		button_event.x     = m_event.button.x;
		button_event.y     = m_event.button.y;

		auto it = mouse_buttons.find(m_event.button.button);
		if (it != mouse_buttons.end())
		{
			button_event.button = it->second;
		}

		if (m_event.type == SDL_MOUSEBUTTONDOWN)
		{
			new_event(MouseButtonDown);
		}
		else
		{
			new_event(MouseButtonUp);
		}
	}

	static void process_event(void (*callback)(const Event& event, void* userdata), void* userdata)
	{
		new (&m_engine_event) Event();
		m_engine_event.window_id = m_event.window.windowID;

		switch (m_event.type)
		{
			case SDL_QUIT:
				new_event(Quit);

			case SDL_APP_TERMINATING:
				new_event(AppTerminating);

			case SDL_APP_LOWMEMORY:
				new_event(Quit);

			case SDL_APP_WILLENTERBACKGROUND:
				new_event(AppPause);

			case SDL_APP_DIDENTERFOREGROUND:
				new_event(AppResume);

			case SDL_DISPLAYEVENT:
			{
				break;
			}

			case SDL_WINDOWEVENT:
			{
				process_window_event(callback, userdata);
				break;
			}

			case SDL_KEYDOWN:
			{
				if (m_event.key.repeat == 0)
				{
					auto& key_event = m_engine_event.keyboard;

					auto it = keys.find(m_event.key.keysym.scancode);
					if (it != keys.end())
					{
						key_event.key = it->second;
						new_event(KeyDown);
					}
					else
					{
						error_log("SDL Window System", "Cannot find scancode '%d'", m_event.key.keysym.scancode);
					}
				}
				break;
			}

			case SDL_KEYUP:
			{
				if (m_event.key.repeat == 0)
				{
					auto& key_event = m_engine_event.keyboard;

					auto it = keys.find(m_event.key.keysym.scancode);
					if (it != keys.end())
					{
						key_event.key = it->second;
						new_event(KeyUp);
					}
					else
					{
						error_log("SDL Window System", "Cannot find scancode '%d'", m_event.key.keysym.scancode);
					}
				}
				break;
			}


			case SDL_MOUSEMOTION:
			{
				int w, h;
				SDL_GetWindowSize(SDL_GetWindowFromID(m_event.window.windowID), &w, &h);

				auto& mouse_motion = m_engine_event.mouse.motion;
				mouse_motion.x     = m_event.motion.x;
				mouse_motion.y     = h - m_event.motion.y;
				mouse_motion.xrel  = m_event.motion.xrel;
				mouse_motion.yrel  = -m_event.motion.yrel;

				new_event(MouseMotion);
			}

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				process_mouse_button(callback, userdata);
			}

			case SDL_MOUSEWHEEL:
			{
				auto& wheel_event = m_engine_event.mouse.wheel;
				wheel_event.x     = m_event.wheel.preciseX;
				wheel_event.y     = m_event.wheel.preciseY;
				new_event(MouseWheel);
			}

			case SDL_TEXTINPUT:
			{
				static_assert(SDL_TEXTINPUTEVENT_TEXT_SIZE == Event::TextInput::max_tex_len + 1);
				std::memcpy(m_engine_event.text_input.text, m_event.text.text, SDL_TEXTINPUTEVENT_TEXT_SIZE);
				new_event(TextInput);
			}

				// case SDL_CONTROLLERDEVICEADDED:
				// {
				//     m_game_controllers[m_event.cdevice.which] = SDL_GameControllerOpen(m_event.cdevice.which);
				//      c_event;
				//     c_event.id = static_cast<Identifier>(m_event.cdevice.which) + 1;
				//     new_event(ControllerDeviceAdded, m_event);
				//     break;
				// }

				// case SDL_CONTROLLERDEVICEREMOVED:
				// {
				//     SDL_GameControllerClose(m_game_controllers[m_event.cdevice.which]);
				//     m_game_controllers.erase(m_event.cdevice.which);
				//     ControllerDeviceAddedEvent c_event;
				//     c_event.id = static_cast<Identifier>(m_event.cdevice.which) + 1;
				//     new_event(ControllerDeviceRemoved, c_event);
				//     break;
				// }

				// case SDL_CONTROLLERAXISMOTION:
				// {
				//     ControllerAxisMotionEvent motion_event;
				//     motion_event.id = static_cast<Identifier>(m_event.caxis.which) + 1;
				//     try
				//     {
				//         motion_event.axis = axis_type.at(m_event.caxis.axis);
				//     }
				//     catch (...)
				//     {
				//         motion_event.axis = GameController::Axis::None;
				//     }

				//     motion_event.value = m_event.caxis.value;
				//     new_event(ControllerAxisMotion, motion_event);
				//     break;
				// }
		}
	}

	static void pool_events_loop(void (*callback)(const Event& event, void* userdata), void* userdata)
	{
		while (SDL_PollEvent(&m_event))
		{
			process_event(callback, userdata);
		}
	}

	ENGINE_EXPORT void pool_events(void (*callback)(const Event& event, void* userdata), void* userdata)
	{
		pool_events_loop(callback, userdata);
	}

	ENGINE_EXPORT void wait_for_events(void (*callback)(const Event& event, void* userdata), void* userdata)
	{
		SDL_WaitEvent(&m_event);
		process_event(callback, userdata);
		pool_events_loop(callback, userdata);
	}
}// namespace Engine::Platform::EventSystem
