#include <Core/logger.hpp>
#include <Core/math/math.hpp>
#include <Systems/Migration/event_system.hpp>
#include <Systems/Migration/input_codes.hpp>
#include <Systems/Migration/input_events.hpp>
#include <Systems/event_system.hpp>
#include <Systems/keyboard_system.hpp>

namespace Trinex
{
	EventSystem* EventSystem::s_instance = nullptr;

	namespace
	{
		static Keyboard::Key legacy_key_of(Migration::ScanCode code)
		{
			switch (code)
			{
				case Migration::ScanCode::Space: return Keyboard::Space;
				case Migration::ScanCode::Apostrophe: return Keyboard::Apostrophe;
				case Migration::ScanCode::Comma: return Keyboard::Comma;
				case Migration::ScanCode::Minus: return Keyboard::Minus;
				case Migration::ScanCode::Period: return Keyboard::Period;
				case Migration::ScanCode::Slash: return Keyboard::Slash;
				case Migration::ScanCode::Num0: return Keyboard::Num0;
				case Migration::ScanCode::Num1: return Keyboard::Num1;
				case Migration::ScanCode::Num2: return Keyboard::Num2;
				case Migration::ScanCode::Num3: return Keyboard::Num3;
				case Migration::ScanCode::Num4: return Keyboard::Num4;
				case Migration::ScanCode::Num5: return Keyboard::Num5;
				case Migration::ScanCode::Num6: return Keyboard::Num6;
				case Migration::ScanCode::Num7: return Keyboard::Num7;
				case Migration::ScanCode::Num8: return Keyboard::Num8;
				case Migration::ScanCode::Num9: return Keyboard::Num9;
				case Migration::ScanCode::Semicolon: return Keyboard::Semicolon;
				case Migration::ScanCode::Equals: return Keyboard::Equal;
				case Migration::ScanCode::A: return Keyboard::A;
				case Migration::ScanCode::B: return Keyboard::B;
				case Migration::ScanCode::C: return Keyboard::C;
				case Migration::ScanCode::D: return Keyboard::D;
				case Migration::ScanCode::E: return Keyboard::E;
				case Migration::ScanCode::F: return Keyboard::F;
				case Migration::ScanCode::G: return Keyboard::G;
				case Migration::ScanCode::H: return Keyboard::H;
				case Migration::ScanCode::I: return Keyboard::I;
				case Migration::ScanCode::J: return Keyboard::J;
				case Migration::ScanCode::K: return Keyboard::K;
				case Migration::ScanCode::L: return Keyboard::L;
				case Migration::ScanCode::M: return Keyboard::M;
				case Migration::ScanCode::N: return Keyboard::N;
				case Migration::ScanCode::O: return Keyboard::O;
				case Migration::ScanCode::P: return Keyboard::P;
				case Migration::ScanCode::Q: return Keyboard::Q;
				case Migration::ScanCode::R: return Keyboard::R;
				case Migration::ScanCode::S: return Keyboard::S;
				case Migration::ScanCode::T: return Keyboard::T;
				case Migration::ScanCode::U: return Keyboard::U;
				case Migration::ScanCode::V: return Keyboard::V;
				case Migration::ScanCode::W: return Keyboard::W;
				case Migration::ScanCode::X: return Keyboard::X;
				case Migration::ScanCode::Y: return Keyboard::Y;
				case Migration::ScanCode::Z: return Keyboard::Z;
				case Migration::ScanCode::LeftBracket: return Keyboard::LeftBracket;
				case Migration::ScanCode::Backslash: return Keyboard::Backslash;
				case Migration::ScanCode::RightBracket: return Keyboard::RightBracket;
				case Migration::ScanCode::Grave: return Keyboard::GraveAccent;
				case Migration::ScanCode::Www: return Keyboard::Explorer;
				case Migration::ScanCode::Escape: return Keyboard::Escape;
				case Migration::ScanCode::Return: return Keyboard::Enter;
				case Migration::ScanCode::Tab: return Keyboard::Tab;
				case Migration::ScanCode::Backspace: return Keyboard::Backspace;
				case Migration::ScanCode::Insert: return Keyboard::Insert;
				case Migration::ScanCode::Delete: return Keyboard::Delete;
				case Migration::ScanCode::Right: return Keyboard::Right;
				case Migration::ScanCode::Left: return Keyboard::Left;
				case Migration::ScanCode::Down: return Keyboard::Down;
				case Migration::ScanCode::Up: return Keyboard::Up;
				case Migration::ScanCode::PageUp: return Keyboard::PageUp;
				case Migration::ScanCode::PageDown: return Keyboard::PageDown;
				case Migration::ScanCode::Home: return Keyboard::Home;
				case Migration::ScanCode::End: return Keyboard::End;
				case Migration::ScanCode::CapsLock: return Keyboard::CapsLock;
				case Migration::ScanCode::ScrollLock: return Keyboard::ScrollLock;
				case Migration::ScanCode::NumLockClear: return Keyboard::NumLock;
				case Migration::ScanCode::PrintScreen: return Keyboard::PrintScreen;
				case Migration::ScanCode::Pause: return Keyboard::Pause;
				case Migration::ScanCode::F1: return Keyboard::F1;
				case Migration::ScanCode::F2: return Keyboard::F2;
				case Migration::ScanCode::F3: return Keyboard::F3;
				case Migration::ScanCode::F4: return Keyboard::F4;
				case Migration::ScanCode::F5: return Keyboard::F5;
				case Migration::ScanCode::F6: return Keyboard::F6;
				case Migration::ScanCode::F7: return Keyboard::F7;
				case Migration::ScanCode::F8: return Keyboard::F8;
				case Migration::ScanCode::F9: return Keyboard::F9;
				case Migration::ScanCode::F10: return Keyboard::F10;
				case Migration::ScanCode::F11: return Keyboard::F11;
				case Migration::ScanCode::F12: return Keyboard::F12;
				case Migration::ScanCode::F13: return Keyboard::F13;
				case Migration::ScanCode::F14: return Keyboard::F14;
				case Migration::ScanCode::F15: return Keyboard::F15;
				case Migration::ScanCode::F16: return Keyboard::F16;
				case Migration::ScanCode::F17: return Keyboard::F17;
				case Migration::ScanCode::F18: return Keyboard::F18;
				case Migration::ScanCode::F19: return Keyboard::F19;
				case Migration::ScanCode::F20: return Keyboard::F20;
				case Migration::ScanCode::F21: return Keyboard::F21;
				case Migration::ScanCode::F22: return Keyboard::F22;
				case Migration::ScanCode::F23: return Keyboard::F23;
				case Migration::ScanCode::F24: return Keyboard::F24;
				case Migration::ScanCode::Kp0: return Keyboard::Kp0;
				case Migration::ScanCode::Kp1: return Keyboard::Kp1;
				case Migration::ScanCode::Kp2: return Keyboard::Kp2;
				case Migration::ScanCode::Kp3: return Keyboard::Kp3;
				case Migration::ScanCode::Kp4: return Keyboard::Kp4;
				case Migration::ScanCode::Kp5: return Keyboard::Kp5;
				case Migration::ScanCode::Kp6: return Keyboard::Kp6;
				case Migration::ScanCode::Kp7: return Keyboard::Kp7;
				case Migration::ScanCode::Kp8: return Keyboard::Kp8;
				case Migration::ScanCode::Kp9: return Keyboard::Kp9;
				case Migration::ScanCode::KpPeriod: return Keyboard::KpDot;
				case Migration::ScanCode::KpDivide: return Keyboard::KpDivide;
				case Migration::ScanCode::KpMultiply: return Keyboard::KpMultiply;
				case Migration::ScanCode::KpMinus: return Keyboard::KpSubtract;
				case Migration::ScanCode::KpPlus: return Keyboard::KpAdd;
				case Migration::ScanCode::KpEnter: return Keyboard::KpEnter;
				case Migration::ScanCode::KpEquals: return Keyboard::KpEqual;
				case Migration::ScanCode::LeftShift: return Keyboard::LeftShift;
				case Migration::ScanCode::LeftControl: return Keyboard::LeftControl;
				case Migration::ScanCode::LeftAlt: return Keyboard::LeftAlt;
				case Migration::ScanCode::LeftGui: return Keyboard::LeftSuper;
				case Migration::ScanCode::RightShift: return Keyboard::RightShift;
				case Migration::ScanCode::RightControl: return Keyboard::RightControl;
				case Migration::ScanCode::RightAlt: return Keyboard::RightAlt;
				case Migration::ScanCode::RightGui: return Keyboard::RightSuper;
				case Migration::ScanCode::Menu: return Keyboard::Menu;
				default: return Keyboard::Unknown;
			}
		}

		static usize encode_utf8(char* output, char32_t cp)
		{
			if (cp <= 0x7F)
			{
				output[0] = static_cast<char>(cp);
				output[1] = '\0';
				return 1;
			}

			if (cp <= 0x7FF)
			{
				output[0] = static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
				output[1] = static_cast<char>(0x80 | (cp & 0x3F));
				output[2] = '\0';
				return 2;
			}

			if (cp <= 0xFFFF)
			{
				output[0] = static_cast<char>(0xE0 | ((cp >> 12) & 0x0F));
				output[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
				output[2] = static_cast<char>(0x80 | (cp & 0x3F));
				output[3] = '\0';
				return 3;
			}

			output[0] = static_cast<char>(0xF0 | ((cp >> 18) & 0x07));
			output[1] = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
			output[2] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
			output[3] = static_cast<char>(0x80 | (cp & 0x3F));
			output[4] = '\0';
			return 4;
		}

		class LegacyEventBridge final : public Migration::EventListener
		{
		public:
			Migration::EventDispatchResult on_event(Migration::RoutedEvent& routed) override
			{
				EventSystem* system = EventSystem::instance();
				if (system == nullptr)
				{
					return {};
				}

				Event event;
				event.window_id = routed.header.window_id;

				switch (routed.header.type_id)
				{
					case Migration::EventTypeIds::Quit: event.type = EventType::Quit; break;

					case Migration::EventTypeIds::Window:
					{
						auto* payload = reinterpret_cast<const Migration::WindowEvent*>(routed.payload);
						if (payload == nullptr)
							return {};

						switch (payload->kind)
						{
							case Migration::WindowEventKind::Shown: event.type = EventType::WindowShown; break;
							case Migration::WindowEventKind::Hidden: event.type = EventType::WindowHidden; break;
							case Migration::WindowEventKind::Moved:
								event.type     = EventType::WindowMoved;
								event.window.x = payload->position.x;
								event.window.y = payload->position.y;
								break;
							case Migration::WindowEventKind::Resized:
								event.type     = EventType::WindowResized;
								event.window.x = payload->size.x;
								event.window.y = payload->size.y;
								break;
							case Migration::WindowEventKind::FocusGained: event.type = EventType::WindowFocusGained; break;
							case Migration::WindowEventKind::FocusLost: event.type = EventType::WindowFocusLost; break;
							case Migration::WindowEventKind::CloseRequested: event.type = EventType::WindowClose; break;
							default: return {};
						}
						break;
					}

					case Migration::EventTypeIds::Key:
					{
						auto* payload = reinterpret_cast<const Migration::KeyEvent*>(routed.payload);
						if (payload == nullptr)
							return {};

						event.type = payload->kind == Migration::KeyEventKind::Released ? EventType::KeyUp : EventType::KeyDown;
						event.keyboard.key = legacy_key_of(static_cast<Migration::ScanCode::Enum>(payload->scan_code));
						break;
					}

					case Migration::EventTypeIds::TextInput:
					{
						auto* payload = reinterpret_cast<const Migration::TextInputEvent*>(routed.payload);
						if (payload == nullptr)
							return {};

						event.type = EventType::TextInput;
						encode_utf8(event.text_input.text, payload->codepoint);
						break;
					}

					case Migration::EventTypeIds::DeviceChange:
					{
						auto* payload = reinterpret_cast<const Migration::DeviceChangeEvent*>(routed.payload);
						if (payload == nullptr || payload->device_type != Migration::InputDeviceType::Gamepad)
							return {};

						switch (payload->kind)
						{
							case Migration::DeviceChangeKind::Added: event.type = EventType::ControllerDeviceAdded; break;
							case Migration::DeviceChangeKind::Removed: event.type = EventType::ControllerDeviceRemoved; break;
							case Migration::DeviceChangeKind::Remapped: event.type = EventType::ControllerDeviceRemapped; break;
							default: return {};
						}
						break;
					}

					default: return {};
				}

				system->push_event(event);
				return {};
			}
		};

		static LegacyEventBridge g_bridge;
		static bool g_bridge_registered = false;
	}// namespace

	EventSystem::EventSystem()
	{
		if (!g_bridge_registered)
		{
			if (Migration::EventSystem* system = Migration::EventSystem::instance())
			{
				system->dispatcher().add_listener(Migration::EventTypeIds::Quit, &g_bridge);
				system->dispatcher().add_listener(Migration::EventTypeIds::Window, &g_bridge);
				system->dispatcher().add_listener(Migration::EventTypeIds::Key, &g_bridge);
				system->dispatcher().add_listener(Migration::EventTypeIds::TextInput, &g_bridge);
				system->dispatcher().add_listener(Migration::EventTypeIds::Pointer, &g_bridge);
				system->dispatcher().add_listener(Migration::EventTypeIds::Gamepad, &g_bridge);
				system->dispatcher().add_listener(Migration::EventTypeIds::DeviceChange, &g_bridge);
				g_bridge_registered = true;
			}
		}
	}

	EventSystem& EventSystem::begin_frame()
	{
		return *this;
	}

	EventSystem& EventSystem::end_frame()
	{
		return *this;
	}

	EventSystem& EventSystem::process_event_method(ProcessEventMethod)
	{
		return *this;
	}

	void EventSystem::dispatch(const Event& event)
	{
		const usize exact_index = static_cast<usize>(event.type);
		const usize any_index   = static_cast<usize>(EventType::Undefined);

		for (const ListenerEntry& entry : m_listeners[exact_index])
		{
			entry.listener(event);
		}

		if (event.type != EventType::Undefined)
		{
			for (const ListenerEntry& entry : m_listeners[any_index])
			{
				entry.listener(event);
			}
		}

		KeyboardSystem::instance()->on_event(event);
	}

	Identifier EventSystem::add_listener(EventType type, const Listener& listener)
	{
		ListenerEntry entry;
		entry.id                   = m_next_listener_id++;
		entry.type                 = type;
		entry.listener             = listener;
		m_listener_types[entry.id] = type;
		m_listeners[static_cast<usize>(type)].push_back(entry);
		return entry.id;
	}

	EventSystem& EventSystem::remove_listener(Identifier id)
	{
		auto found = m_listener_types.find(id);
		if (found == m_listener_types.end())
			return *this;

		auto& list = m_listeners[static_cast<usize>(found->second)];
		for (auto it = list.begin(); it != list.end(); ++it)
		{
			if (it->id == id)
			{
				list.erase(it);
				break;
			}
		}

		m_listener_types.erase(found);
		return *this;
	}

	EventSystem& EventSystem::push_event(const Event& event)
	{
		dispatch(event);
		return *this;
	}
}// namespace Trinex
