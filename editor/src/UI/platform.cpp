#include <Core/base_engine.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/class.hpp>
#include <Core/window.hpp>
#include <Graphics/render_viewport.hpp>
#include <Input/event_system.hpp>
#include <Input/input_codes.hpp>
#include <Input/input_events.hpp>
#include <Platform/platform.hpp>
#include <UI/client.hpp>
#include <imgui.h>

namespace Trinex
{
	namespace
	{
		namespace WindowBackend
		{
			struct ImGuiTrinexWindowData {
				float time;
				bool update_monitors;

				ImGuiTrinexWindowData() : time(engine_instance->time_seconds()), update_monitors(true) {}
			};

			struct ImGuiTrinexViewportData {
				Window* window    = nullptr;
				ImGuiContext* ctx = nullptr;
				bool owns_window  = false;
			};

			static Map<Window*, ImGuiViewport*> s_viewports;

			static ImGuiTrinexWindowData* backend_data()
			{
				return ImGui::GetCurrentContext()
				               ? reinterpret_cast<ImGuiTrinexWindowData*>(ImGui::GetIO().BackendPlatformUserData)
				               : nullptr;
			}

			static ImGuiTrinexViewportData* viewport_data(ImGuiViewport* vp)
			{
				return vp ? reinterpret_cast<ImGuiTrinexViewportData*>(vp->PlatformUserData) : nullptr;
			}

			static ImVec2 trinex_to_imgui_pos(Window* window)
			{
				auto pos  = window->position();
				auto info = Platform::monitor_info(window->monitor_index());
				return {static_cast<float>(pos.x), static_cast<float>(info.size.y - (pos.y + window->size().y))};
			}

			static Vector2u imgui_to_trinex_pos(Window* window, ImVec2 pos)
			{
				auto info = Platform::monitor_info(window->monitor_index());
				return {static_cast<u32>(pos.x), static_cast<u32>(info.size.y - (pos.y + window->size().y))};
			}

			static ImVec2 mouse_to_imgui_pos(Window* window, float x, float y)
			{
				if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				{
					auto pos = trinex_to_imgui_pos(window);
					return {x + pos.x, window->size().y - y + pos.y};
				}

				return {x, window->size().y - y};
			}

			static float window_dpi_scale(Window* window)
			{
				float dpi = Platform::monitor_info(window->monitor_index()).dpi;
				return dpi > 0.f ? dpi / 96.0f : 1.f;
			}

			struct ImGuiContextSaver {
				ImGuiContext* ctx;

				ImGuiContextSaver(ImGuiContext* new_context) : ctx(ImGui::GetCurrentContext())
				{
					ImGui::SetCurrentContext(new_context);
				}

				~ImGuiContextSaver() { ImGui::SetCurrentContext(ctx); }
			};

			static ImGuiMouseButton imgui_mouse_button_of(MouseButton button)
			{
				switch (button)
				{
					case MouseButton::Left: return ImGuiMouseButton_Left;
					case MouseButton::Middle: return ImGuiMouseButton_Middle;
					case MouseButton::Right: return ImGuiMouseButton_Right;
					case MouseButton::X1:
					case MouseButton::X2:
					default: return -1;
				}
			}

			static ImGuiKey imgui_key_of(ScanCode button)
			{
				switch (button)
				{
					case ScanCode::Space: return ImGuiKey_Space;
					case ScanCode::Apostrophe: return ImGuiKey_Apostrophe;
					case ScanCode::Comma: return ImGuiKey_Comma;
					case ScanCode::Minus: return ImGuiKey_Minus;
					case ScanCode::Period: return ImGuiKey_Period;
					case ScanCode::Slash: return ImGuiKey_Slash;
					case ScanCode::Num0: return ImGuiKey_0;
					case ScanCode::Num1: return ImGuiKey_1;
					case ScanCode::Num2: return ImGuiKey_2;
					case ScanCode::Num3: return ImGuiKey_3;
					case ScanCode::Num4: return ImGuiKey_4;
					case ScanCode::Num5: return ImGuiKey_5;
					case ScanCode::Num6: return ImGuiKey_6;
					case ScanCode::Num7: return ImGuiKey_7;
					case ScanCode::Num8: return ImGuiKey_8;
					case ScanCode::Num9: return ImGuiKey_9;
					case ScanCode::Semicolon: return ImGuiKey_Semicolon;
					case ScanCode::Equals: return ImGuiKey_Equal;
					case ScanCode::A: return ImGuiKey_A;
					case ScanCode::B: return ImGuiKey_B;
					case ScanCode::C: return ImGuiKey_C;
					case ScanCode::D: return ImGuiKey_D;
					case ScanCode::E: return ImGuiKey_E;
					case ScanCode::F: return ImGuiKey_F;
					case ScanCode::G: return ImGuiKey_G;
					case ScanCode::H: return ImGuiKey_H;
					case ScanCode::I: return ImGuiKey_I;
					case ScanCode::J: return ImGuiKey_J;
					case ScanCode::K: return ImGuiKey_K;
					case ScanCode::L: return ImGuiKey_L;
					case ScanCode::M: return ImGuiKey_M;
					case ScanCode::N: return ImGuiKey_N;
					case ScanCode::O: return ImGuiKey_O;
					case ScanCode::P: return ImGuiKey_P;
					case ScanCode::Q: return ImGuiKey_Q;
					case ScanCode::R: return ImGuiKey_R;
					case ScanCode::S: return ImGuiKey_S;
					case ScanCode::T: return ImGuiKey_T;
					case ScanCode::U: return ImGuiKey_U;
					case ScanCode::V: return ImGuiKey_V;
					case ScanCode::W: return ImGuiKey_W;
					case ScanCode::X: return ImGuiKey_X;
					case ScanCode::Y: return ImGuiKey_Y;
					case ScanCode::Z: return ImGuiKey_Z;
					case ScanCode::LeftBracket: return ImGuiKey_LeftBracket;
					case ScanCode::Backslash: return ImGuiKey_Backslash;
					case ScanCode::RightBracket: return ImGuiKey_RightBracket;
					case ScanCode::Grave: return ImGuiKey_GraveAccent;
					case ScanCode::Www: return ImGuiKey_None;
					case ScanCode::Escape: return ImGuiKey_Escape;
					case ScanCode::Return: return ImGuiKey_Enter;
					case ScanCode::Tab: return ImGuiKey_Tab;
					case ScanCode::Backspace: return ImGuiKey_Backspace;
					case ScanCode::Insert: return ImGuiKey_Insert;
					case ScanCode::Delete: return ImGuiKey_Delete;
					case ScanCode::Right: return ImGuiKey_RightArrow;
					case ScanCode::Left: return ImGuiKey_LeftArrow;
					case ScanCode::Down: return ImGuiKey_DownArrow;
					case ScanCode::Up: return ImGuiKey_UpArrow;
					case ScanCode::PageUp: return ImGuiKey_PageUp;
					case ScanCode::PageDown: return ImGuiKey_PageDown;
					case ScanCode::Home: return ImGuiKey_Home;
					case ScanCode::End: return ImGuiKey_End;
					case ScanCode::CapsLock: return ImGuiKey_CapsLock;
					case ScanCode::ScrollLock: return ImGuiKey_ScrollLock;
					case ScanCode::NumLockClear: return ImGuiKey_NumLock;
					case ScanCode::PrintScreen: return ImGuiKey_PrintScreen;
					case ScanCode::Pause: return ImGuiKey_Pause;
					case ScanCode::F1: return ImGuiKey_F1;
					case ScanCode::F2: return ImGuiKey_F2;
					case ScanCode::F3: return ImGuiKey_F3;
					case ScanCode::F4: return ImGuiKey_F4;
					case ScanCode::F5: return ImGuiKey_F5;
					case ScanCode::F6: return ImGuiKey_F6;
					case ScanCode::F7: return ImGuiKey_F7;
					case ScanCode::F8: return ImGuiKey_F8;
					case ScanCode::F9: return ImGuiKey_F9;
					case ScanCode::F10: return ImGuiKey_F10;
					case ScanCode::F11: return ImGuiKey_F11;
					case ScanCode::F12: return ImGuiKey_F12;
					case ScanCode::F13: return ImGuiKey_F13;
					case ScanCode::F14: return ImGuiKey_F14;
					case ScanCode::F15: return ImGuiKey_F15;
					case ScanCode::F16: return ImGuiKey_F16;
					case ScanCode::F17: return ImGuiKey_F17;
					case ScanCode::F18: return ImGuiKey_F18;
					case ScanCode::F19: return ImGuiKey_F19;
					case ScanCode::F20: return ImGuiKey_F20;
					case ScanCode::F21: return ImGuiKey_F21;
					case ScanCode::F22: return ImGuiKey_F22;
					case ScanCode::F23: return ImGuiKey_F23;
					case ScanCode::F24: return ImGuiKey_F24;
					case ScanCode::Kp0: return ImGuiKey_Keypad0;
					case ScanCode::Kp1: return ImGuiKey_Keypad1;
					case ScanCode::Kp2: return ImGuiKey_Keypad2;
					case ScanCode::Kp3: return ImGuiKey_Keypad3;
					case ScanCode::Kp4: return ImGuiKey_Keypad4;
					case ScanCode::Kp5: return ImGuiKey_Keypad5;
					case ScanCode::Kp6: return ImGuiKey_Keypad6;
					case ScanCode::Kp7: return ImGuiKey_Keypad7;
					case ScanCode::Kp8: return ImGuiKey_Keypad8;
					case ScanCode::Kp9: return ImGuiKey_Keypad9;
					case ScanCode::KpPeriod: return ImGuiKey_KeypadDecimal;
					case ScanCode::KpDivide: return ImGuiKey_KeypadDivide;
					case ScanCode::KpMultiply: return ImGuiKey_KeypadMultiply;
					case ScanCode::KpMinus: return ImGuiKey_KeypadSubtract;
					case ScanCode::KpPlus: return ImGuiKey_KeypadAdd;
					case ScanCode::KpEnter: return ImGuiKey_KeypadEnter;
					case ScanCode::KpEquals: return ImGuiKey_KeypadEqual;
					case ScanCode::LeftShift: return ImGuiKey_LeftShift;
					case ScanCode::LeftControl: return ImGuiKey_LeftCtrl;
					case ScanCode::LeftAlt: return ImGuiKey_LeftAlt;
					case ScanCode::LeftGui: return ImGuiKey_LeftSuper;
					case ScanCode::RightShift: return ImGuiKey_RightShift;
					case ScanCode::RightControl: return ImGuiKey_RightCtrl;
					case ScanCode::RightAlt: return ImGuiKey_RightAlt;
					case ScanCode::RightGui: return ImGuiKey_RightSuper;
					case ScanCode::Menu: return ImGuiKey_Menu;
					default: return ImGuiKey_None;
				}
			}

			template<typename F>
			static void with_window_context(Identifier window_id, F&& f)
			{
				Trinex::Window* window = Window::find(window_id);
				if (window == nullptr)
					return;

				auto it = s_viewports.find(window);
				if (it == s_viewports.end())
					return;

				ImGuiTrinexViewportData* data = viewport_data(it->second);

				if (data && data->ctx)
				{
					ImGuiContextSaver saver(data->ctx);
					f(window);
				}
			}

			static void imgui_sent_mouse_position(Trinex::Window* window, float x, float y)
			{
				auto& io   = ImGui::GetIO();
				ImVec2 pos = mouse_to_imgui_pos(window, x, y);
				io.AddMousePosEvent(pos.x, pos.y);
			}

			static void on_mouse_move(Identifier window_id, const PointerEvent& data)
			{
				with_window_context(window_id, [&data](Trinex::Window* window) {
					auto& io = ImGui::GetIO();
					io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
					imgui_sent_mouse_position(window, data.screen_position.x, data.screen_position.y);
				});
			}

			static void on_mouse_button(Identifier window_id, MouseButton button, bool is_pressed)
			{
				auto imgui_button = imgui_mouse_button_of(button);

				if (imgui_button != -1)
				{
					with_window_context(window_id, [is_pressed, imgui_button](Trinex::Window*) {
						auto& io = ImGui::GetIO();
						io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
						io.AddMouseButtonEvent(imgui_button, is_pressed);
					});
				}
			}

			static void on_mouse_wheel(Identifier window_id, const PointerEvent& data)
			{
				with_window_context(window_id, [&data](Trinex::Window*) {
					auto& io = ImGui::GetIO();
					io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
					io.AddMouseWheelEvent(data.wheel_delta.x, data.wheel_delta.y);
				});
			}

			static void on_keyboard_button(Identifier window_id, ScanCode button, bool is_pressed)
			{
				auto imgui_button = imgui_key_of(button);

				if (imgui_button != ImGuiKey_None)
				{
					with_window_context(window_id, [is_pressed, imgui_button](Trinex::Window*) {
						auto& io = ImGui::GetIO();

						if (is_in<ImGuiKey_LeftCtrl, ImGuiKey_RightCtrl>(imgui_button))
							io.AddKeyEvent(ImGuiMod_Ctrl, is_pressed);
						if (is_in<ImGuiKey_LeftShift, ImGuiKey_RightShift>(imgui_button))
							io.AddKeyEvent(ImGuiMod_Shift, is_pressed);
						if (is_in<ImGuiKey_LeftAlt, ImGuiKey_RightAlt>(imgui_button))
							io.AddKeyEvent(ImGuiMod_Alt, is_pressed);
						if (is_in<ImGuiKey_LeftSuper, ImGuiKey_RightSuper>(imgui_button))
							io.AddKeyEvent(ImGuiMod_Super, is_pressed);
						io.AddKeyEvent(imgui_button, is_pressed);
					});
				}
			}

			static void on_text_input(Identifier window_id, const TextInputEvent& data)
			{
				with_window_context(window_id, [&data](Trinex::Window*) {
					auto& io = ImGui::GetIO();
					io.AddInputCharacter(static_cast<unsigned int>(data.codepoint));
				});
			}

			static void on_window_close(Identifier window_id)
			{
				with_window_context(window_id, [](Trinex::Window* window) {
					if (auto it = s_viewports.find(window); it != s_viewports.end())
					{
						it->second->PlatformRequestClose = true;
					}
				});
			}

			static void on_window_move(Identifier window_id)
			{
				with_window_context(window_id, [](Trinex::Window* window) {
					if (auto it = s_viewports.find(window); it != s_viewports.end())
					{
						it->second->PlatformRequestMove = true;
					}
				});
			}

			static void on_window_resize(Identifier window_id)
			{
				with_window_context(window_id, [](Trinex::Window* window) {
					if (auto it = s_viewports.find(window); it != s_viewports.end())
					{
						it->second->PlatformRequestResize = true;
					}
				});
			}

			class ImGuiEventListener final : public EventListener
			{
			public:
				EventDispatchResult on_event(RoutedEvent& event) override
				{
					switch (event.header.type_id)
					{
						case EventTypeIds::Pointer:
						{
							auto* data = reinterpret_cast<const PointerEvent*>(event.payload);
							if (data == nullptr)
								return {};

							switch (data->kind)
							{
								case PointerEventKind::Moved: on_mouse_move(event.header.window_id, *data); break;
								case PointerEventKind::ButtonPressed:
									on_mouse_button(event.header.window_id, static_cast<MouseButton::Enum>(data->button), true);
									break;
								case PointerEventKind::ButtonReleased:
									on_mouse_button(event.header.window_id, static_cast<MouseButton::Enum>(data->button), false);
									break;
								case PointerEventKind::Wheel: on_mouse_wheel(event.header.window_id, *data); break;
								default: break;
							}
							break;
						}

						case EventTypeIds::Key:
						{
							auto* data = reinterpret_cast<const KeyEvent*>(event.payload);
							if (data)
							{
								on_keyboard_button(event.header.window_id, static_cast<ScanCode::Enum>(data->scan_code),
								                   data->kind != KeyEventKind::Released);
							}
							break;
						}

						case EventTypeIds::TextInput:
						{
							auto* data = reinterpret_cast<const TextInputEvent*>(event.payload);
							if (data)
							{
								on_text_input(event.header.window_id, *data);
							}
							break;
						}

						case EventTypeIds::Window:
						{
							auto* data = reinterpret_cast<const WindowEvent*>(event.payload);
							if (data == nullptr)
								return {};

							switch (data->kind)
							{
								case WindowEventKind::CloseRequested: on_window_close(event.header.window_id); break;
								case WindowEventKind::Moved: on_window_move(event.header.window_id); break;
								case WindowEventKind::Resized: on_window_resize(event.header.window_id); break;
								default: break;
							}
							break;
						}

						default: break;
					}

					return {};
				}
			};

			static ImGuiEventListener m_listener;
			static bool m_listener_enabled = false;

			void disable_events()
			{
				if (!m_listener_enabled)
					return;

				if (EventSystem* system = EventSystem::instance())
				{
					system->dispatcher().remove_listener(EventTypeIds::Pointer, &m_listener);
					system->dispatcher().remove_listener(EventTypeIds::Key, &m_listener);
					system->dispatcher().remove_listener(EventTypeIds::TextInput, &m_listener);
					system->dispatcher().remove_listener(EventTypeIds::Window, &m_listener);
				}

				m_listener_enabled = false;
			}

			void enable_events()
			{
				if (m_listener_enabled)
					return;

				if (EventSystem* system = EventSystem::instance())
				{
					system->dispatcher().add_listener(EventTypeIds::Pointer, &m_listener);
					system->dispatcher().add_listener(EventTypeIds::Key, &m_listener);
					system->dispatcher().add_listener(EventTypeIds::TextInput, &m_listener);
					system->dispatcher().add_listener(EventTypeIds::Window, &m_listener);
					m_listener_enabled = true;
				}
			}

			static FORCE_INLINE Trinex::Window* window_from(ImGuiViewport* vp)
			{
				ImGuiTrinexViewportData* data = viewport_data(vp);
				return data ? data->window : nullptr;
			}

			static void window_create(ImGuiViewport* vp)
			{
				WindowDesc desc = {};
				desc.pos.x      = vp->Pos.x;
				desc.pos.y      = vp->Pos.y;
				desc.size.x     = vp->Size.x;
				desc.size.y     = vp->Size.y;

				desc.attributes = WindowAttribute::Hidden;

				if (vp->Flags & ImGuiViewportFlags_NoDecoration)
				{
					desc.attributes.set(WindowAttribute::BorderLess);
				}
				else
				{
					desc.attributes.set(WindowAttribute::Resizable);
				}

				auto parent_window =
				        vp->ParentViewportId != 0 ? window_from(ImGui::FindViewportByID(vp->ParentViewportId)) : nullptr;
				auto new_window   = Trinex::Window::create(desc, parent_window);
				auto data         = IM_NEW(ImGuiTrinexViewportData)();
				data->window      = new_window;
				data->ctx         = ImGui::GetCurrentContext();
				data->owns_window = true;

				vp->PlatformHandle      = new_window;
				vp->PlatformUserData    = data;
				s_viewports[new_window] = vp;

				new_window->on_destroy.push([](Window* window) {
					auto it = s_viewports.find(window);
					if (it == s_viewports.end())
						return;

					ImGuiViewport* vp = it->second;
					if (ImGuiTrinexViewportData* data = viewport_data(vp))
					{
						data->window      = nullptr;
						data->owns_window = false;
					}

					vp->PlatformHandle = nullptr;
					s_viewports.erase(it);
				});
			}

			static void window_destroy(ImGuiViewport* vp)
			{
				ImGuiTrinexViewportData* data = viewport_data(vp);
				Window* window                = data ? data->window : nullptr;

				if (data && data->owns_window && window)
				{
					data->owns_window = false;
					Window::destroy(window);
				}

				if (window)
					s_viewports.erase(window);

				if (data)
					IM_DELETE(data);

				vp->PlatformUserData = nullptr;
				vp->PlatformHandle   = nullptr;
			}

			static void window_show(ImGuiViewport* vp)
			{
				if (Trinex::Window* wd = window_from(vp))
				{
					wd->show();
				}
			}

			static void set_window_pos(ImGuiViewport* vp, ImVec2 pos)
			{
				if (Trinex::Window* wd = window_from(vp))
				{
					wd->position(imgui_to_trinex_pos(wd, pos));
				}
			}

			static ImVec2 get_window_pos(ImGuiViewport* vp)
			{
				if (Trinex::Window* wd = window_from(vp))
				{
					return trinex_to_imgui_pos(wd);
				}

				return {0, 0};
			}

			static void set_window_size(ImGuiViewport* vp, ImVec2 size)
			{
				if (Trinex::Window* wd = window_from(vp))
				{
					wd->size({size.x, size.y});
				}
			}

			static ImVec2 get_window_size(ImGuiViewport* vp)
			{
				if (Trinex::Window* wd = window_from(vp))
				{
					auto size = wd->size();
					return {static_cast<float>(size.x), static_cast<float>(size.y)};
				}

				return {0, 0};
			}

			static ImVec2 get_window_framebuffer_scale(ImGuiViewport* vp)
			{
				if (Trinex::Window* wd = window_from(vp))
				{
					float scale = window_dpi_scale(wd);
					return {scale, scale};
				}

				return {1.f, 1.f};
			}

			static bool get_window_focus(ImGuiViewport* vp)
			{
				if (Trinex::Window* wd = window_from(vp))
				{
					return wd->focused();
				}
				return false;
			}

			static void set_window_focus(ImGuiViewport* vp)
			{
				if (Trinex::Window* wd = window_from(vp))
				{
					wd->focus();
				}
			}

			static void set_window_title(ImGuiViewport* vp, const char* title)
			{
				if (Trinex::Window* wd = window_from(vp))
				{
					wd->title(title);
				}
			}

			static bool get_window_minimized(ImGuiViewport* vp)
			{
				if (Trinex::Window* wd = window_from(vp))
				{
					return wd->is_iconify();
				}
				return false;
			}

			static void init_platform_interface(Trinex::Window* window)
			{
				ImGuiPlatformIO& platform_io                   = ImGui::GetPlatformIO();
				platform_io.Platform_CreateWindow              = window_create;
				platform_io.Platform_DestroyWindow             = window_destroy;
				platform_io.Platform_ShowWindow                = window_show;
				platform_io.Platform_SetWindowPos              = set_window_pos;
				platform_io.Platform_GetWindowPos              = get_window_pos;
				platform_io.Platform_SetWindowSize             = set_window_size;
				platform_io.Platform_GetWindowSize             = get_window_size;
				platform_io.Platform_GetWindowFramebufferScale = get_window_framebuffer_scale;
				platform_io.Platform_SetWindowFocus            = set_window_focus;
				platform_io.Platform_GetWindowFocus            = get_window_focus;
				platform_io.Platform_GetWindowMinimized        = get_window_minimized;
				platform_io.Platform_SetWindowTitle            = set_window_title;

				ImGuiViewport* main_viewport    = ImGui::GetMainViewport();
				auto data                       = IM_NEW(ImGuiTrinexViewportData)();
				data->window                    = window;
				data->ctx                       = ImGui::GetCurrentContext();
				data->owns_window               = false;
				main_viewport->PlatformHandle   = window;
				main_viewport->PlatformUserData = data;
				s_viewports[window]             = main_viewport;
			}

			static void update_monitors()
			{
				auto* bd                     = backend_data();
				ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
				platform_io.Monitors.resize(0);
				bd->update_monitors = false;

				usize display_count = Platform::monitors_count();
				for (usize n = 0; n < display_count; n++)
				{
					ImGuiPlatformMonitor monitor;
					auto info       = Platform::monitor_info(n);
					monitor.WorkPos = monitor.MainPos = ImVec2(info.pos.x, info.pos.y);
					monitor.WorkSize = monitor.MainSize = ImVec2(info.size.x, info.size.y);
					monitor.DpiScale                    = info.dpi / 96.0f;

					monitor.PlatformHandle = reinterpret_cast<void*>(n);
					platform_io.Monitors.push_back(monitor);
				}
			}

			static class PlatformListener : public UI::ClientListener
			{
			public:
				PlatformListener() : UI::ClientListener(0) {}

				ClientListener& on_create(UI::Client* client) override
				{
					auto window = client->viewport()->window();

					ImGuiIO& io            = ImGui::GetIO();
					io.BackendPlatformName = "imgui_impl_trinex";
					auto bd                = IM_NEW(ImGuiTrinexWindowData)();

					enable_events();
					io.BackendPlatformUserData = bd;
					io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;


					if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) &&
					    (io.BackendFlags & ImGuiBackendFlags_PlatformHasViewports))
						init_platform_interface(window);
					return *this;
				}

				ClientListener& on_destroy(UI::Client* client) override
				{
					ImGui::DestroyPlatformWindows();
					if (ImGuiTrinexViewportData* data = viewport_data(ImGui::GetMainViewport()))
					{
						if (data->window)
							s_viewports.erase(data->window);

						IM_DELETE(data);
						ImGui::GetMainViewport()->PlatformUserData = nullptr;
						ImGui::GetMainViewport()->PlatformHandle   = nullptr;
					}

					ImGuiIO& io            = ImGui::GetIO();
					io.BackendPlatformName = nullptr;
					IM_DELETE(reinterpret_cast<ImGuiTrinexWindowData*>(io.BackendPlatformUserData));
					io.BackendPlatformUserData = nullptr;

					return *this;
				}

				ClientListener& on_begin_frame(UI::Client* client) override
				{
					Window* window = client->viewport()->window();

					ImGuiIO& io = ImGui::GetIO();
					auto bd     = backend_data();

					Vector2f size          = Vector2f(window->size());
					Vector2f drawable_size = Vector2f(window->size());

					io.DisplaySize = ImVec2(size.x, size.y);
					if (drawable_size.x > 0 && drawable_size.y > 0)
						io.DisplayFramebufferScale = ImVec2(drawable_size.x / size.x, drawable_size.y / size.y);

					// Update monitors
					if (bd->update_monitors)
						update_monitors();

					float current_time = engine_instance->time_seconds();
					io.DeltaTime       = bd->time > 0.0 ? (current_time - bd->time) : (1.0f / 60.0f);
					bd->time           = current_time;
					return *this;
				}
			} s_listener;
		}// namespace WindowBackend
	}// namespace
}// namespace Trinex
