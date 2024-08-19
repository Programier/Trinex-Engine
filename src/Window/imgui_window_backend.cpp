#include "Core/object.hpp"
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/threading.hpp>
#include <Event/event.hpp>
#include <Event/event_data.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/rhi.hpp>
#include <Platform/platform.hpp>
#include <Systems/event_system.hpp>
#include <Systems/keyboard_system.hpp>
#include <Systems/mouse_system.hpp>
#include <Window/config.hpp>
#include <Window/imgui_window_backend.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
	namespace ImGuiBackend
	{
		extern void imgui_trinex_rhi_render_draw_data(ImGuiContext* ctx, ImDrawData* draw_data);
	}

	namespace ImGuiWindowBackend
	{
		struct ImGuiTrinexWindowData {
			Window* window;
			float time;
			bool update_monitors;

			ImGuiTrinexWindowData() : window(nullptr), time(engine_instance->time_seconds()), update_monitors(true)
			{}
		};

		class ENGINE_EXPORT ImGuiViewportClient : public ViewportClient
		{
			declare_class(ImGuiViewportClient, ViewportClient);
			ImGuiRenderer::DrawData m_draw_data;
			class ImGuiRenderer::Window* m_window;

		public:
			ImGuiViewport* viewport = nullptr;
			ImGuiContext* context;

			ViewportClient& on_bind_viewport(class RenderViewport* viewport) override
			{
				m_window = ImGuiRenderer::Window::current();
				return *this;
			}

			ViewportClient& render(class RenderViewport* viewport) override
			{
				viewport->rhi_bind();
				rhi->push_debug_stage("ImGuiViewportClient");
				ImGuiBackend::imgui_trinex_rhi_render_draw_data(m_window->context(), m_draw_data.draw_data());
				m_draw_data.swap_render_index();
				rhi->pop_debug_stage();
				return *this;
			}

			ViewportClient& update(class RenderViewport*, float dt) override
			{
				m_draw_data.copy(viewport->DrawData);
				m_draw_data.swap_logic_index();
				return *this;
			}
		};

		implement_class_default_init(Engine::ImGuiBackend, ImGuiViewportClient, 0);

		static ImGuiTrinexWindowData* imgui_trinex_backend_data()
		{
			return ImGui::GetCurrentContext() ? reinterpret_cast<ImGuiTrinexWindowData*>(ImGui::GetIO().BackendPlatformUserData)
											  : nullptr;
		}

		static FORCE_INLINE Window* window_from(const Event& event)
		{
			return WindowManager::instance()->find(event.window_id());
		}

		struct ImGuiContextSaver {
			ImGuiContext* ctx;

			ImGuiContextSaver(ImGuiContext* new_context) : ctx(ImGui::GetCurrentContext())
			{
				ImGui::SetCurrentContext(new_context);
			}

			~ImGuiContextSaver()
			{
				ImGui::SetCurrentContext(ctx);
			}
		};


		static ImGuiMouseButton imgui_button_of(Mouse::Button button)
		{
			switch (button)
			{
				case Mouse::Button::Left:
					return ImGuiMouseButton_Left;
				case Mouse::Button::Middle:
					return ImGuiMouseButton_Middle;
				case Mouse::Button::Right:
					return ImGuiMouseButton_Right;
				case Mouse::Button::Forward:
				case Mouse::Button::Back:
				default:
					return -1;
			}
		}

		static ImGuiKey imgui_button_of(Keyboard::Key button)
		{
			switch (button)
			{
				case Keyboard::Key::Space:
					return ImGuiKey_Space;
				case Keyboard::Key::Apostrophe:
					return ImGuiKey_Apostrophe;
				case Keyboard::Key::Comma:
					return ImGuiKey_Comma;
				case Keyboard::Key::Minus:
					return ImGuiKey_Minus;
				case Keyboard::Key::Period:
					return ImGuiKey_Period;
				case Keyboard::Key::Slash:
					return ImGuiKey_Slash;
				case Keyboard::Key::Num0:
					return ImGuiKey_0;
				case Keyboard::Key::Num1:
					return ImGuiKey_1;
				case Keyboard::Key::Num2:
					return ImGuiKey_2;
				case Keyboard::Key::Num3:
					return ImGuiKey_3;
				case Keyboard::Key::Num4:
					return ImGuiKey_4;
				case Keyboard::Key::Num5:
					return ImGuiKey_5;
				case Keyboard::Key::Num6:
					return ImGuiKey_6;
				case Keyboard::Key::Num7:
					return ImGuiKey_7;
				case Keyboard::Key::Num8:
					return ImGuiKey_8;
				case Keyboard::Key::Num9:
					return ImGuiKey_9;
				case Keyboard::Key::Semicolon:
					return ImGuiKey_Semicolon;
				case Keyboard::Key::Equal:
					return ImGuiKey_Equal;
				case Keyboard::Key::A:
					return ImGuiKey_A;
				case Keyboard::Key::B:
					return ImGuiKey_B;
				case Keyboard::Key::C:
					return ImGuiKey_C;
				case Keyboard::Key::D:
					return ImGuiKey_D;
				case Keyboard::Key::E:
					return ImGuiKey_E;
				case Keyboard::Key::F:
					return ImGuiKey_F;
				case Keyboard::Key::G:
					return ImGuiKey_G;
				case Keyboard::Key::H:
					return ImGuiKey_H;
				case Keyboard::Key::I:
					return ImGuiKey_I;
				case Keyboard::Key::J:
					return ImGuiKey_J;
				case Keyboard::Key::K:
					return ImGuiKey_K;
				case Keyboard::Key::L:
					return ImGuiKey_L;
				case Keyboard::Key::M:
					return ImGuiKey_M;
				case Keyboard::Key::N:
					return ImGuiKey_N;
				case Keyboard::Key::O:
					return ImGuiKey_O;
				case Keyboard::Key::P:
					return ImGuiKey_P;
				case Keyboard::Key::Q:
					return ImGuiKey_Q;
				case Keyboard::Key::R:
					return ImGuiKey_R;
				case Keyboard::Key::S:
					return ImGuiKey_S;
				case Keyboard::Key::T:
					return ImGuiKey_T;
				case Keyboard::Key::U:
					return ImGuiKey_U;
				case Keyboard::Key::V:
					return ImGuiKey_V;
				case Keyboard::Key::W:
					return ImGuiKey_W;
				case Keyboard::Key::X:
					return ImGuiKey_X;
				case Keyboard::Key::Y:
					return ImGuiKey_Y;
				case Keyboard::Key::Z:
					return ImGuiKey_Z;
				case Keyboard::Key::LeftBracket:
					return ImGuiKey_LeftBracket;
				case Keyboard::Key::Backslash:
					return ImGuiKey_Backslash;
				case Keyboard::Key::RightBracket:
					return ImGuiKey_RightBracket;
				case Keyboard::Key::GraveAccent:
					return ImGuiKey_GraveAccent;
				case Keyboard::Key::Explorer:
					return ImGuiKey_None;
				case Keyboard::Key::Escape:
					return ImGuiKey_Escape;
				case Keyboard::Key::Enter:
					return ImGuiKey_Enter;
				case Keyboard::Key::Tab:
					return ImGuiKey_Tab;
				case Keyboard::Key::Backspace:
					return ImGuiKey_Backspace;
				case Keyboard::Key::Insert:
					return ImGuiKey_Insert;
				case Keyboard::Key::Delete:
					return ImGuiKey_Delete;
				case Keyboard::Key::Right:
					return ImGuiKey_RightArrow;
				case Keyboard::Key::Left:
					return ImGuiKey_LeftArrow;
				case Keyboard::Key::Down:
					return ImGuiKey_DownArrow;
				case Keyboard::Key::Up:
					return ImGuiKey_UpArrow;
				case Keyboard::Key::PageUp:
					return ImGuiKey_PageUp;
				case Keyboard::Key::PageDown:
					return ImGuiKey_PageDown;
				case Keyboard::Key::Home:
					return ImGuiKey_Home;
				case Keyboard::Key::End:
					return ImGuiKey_End;
				case Keyboard::Key::CapsLock:
					return ImGuiKey_CapsLock;
				case Keyboard::Key::ScrollLock:
					return ImGuiKey_ScrollLock;
				case Keyboard::Key::NumLock:
					return ImGuiKey_NumLock;
				case Keyboard::Key::PrintScreen:
					return ImGuiKey_PrintScreen;
				case Keyboard::Key::Pause:
					return ImGuiKey_Pause;
				case Keyboard::Key::F1:
					return ImGuiKey_F1;
				case Keyboard::Key::F2:
					return ImGuiKey_F2;
				case Keyboard::Key::F3:
					return ImGuiKey_F3;
				case Keyboard::Key::F4:
					return ImGuiKey_F4;
				case Keyboard::Key::F5:
					return ImGuiKey_F5;
				case Keyboard::Key::F6:
					return ImGuiKey_F6;
				case Keyboard::Key::F7:
					return ImGuiKey_F7;
				case Keyboard::Key::F8:
					return ImGuiKey_F8;
				case Keyboard::Key::F9:
					return ImGuiKey_F9;
				case Keyboard::Key::F10:
					return ImGuiKey_F10;
				case Keyboard::Key::F11:
					return ImGuiKey_F11;
				case Keyboard::Key::F12:
					return ImGuiKey_F12;
				case Keyboard::Key::F13:
					return ImGuiKey_F13;
				case Keyboard::Key::F14:
					return ImGuiKey_F14;
				case Keyboard::Key::F15:
					return ImGuiKey_F15;
				case Keyboard::Key::F16:
					return ImGuiKey_F16;
				case Keyboard::Key::F17:
					return ImGuiKey_F17;
				case Keyboard::Key::F18:
					return ImGuiKey_F18;
				case Keyboard::Key::F19:
					return ImGuiKey_F19;
				case Keyboard::Key::F20:
					return ImGuiKey_F20;
				case Keyboard::Key::F21:
					return ImGuiKey_F21;
				case Keyboard::Key::F22:
					return ImGuiKey_F22;
				case Keyboard::Key::F23:
					return ImGuiKey_F23;
				case Keyboard::Key::F24:
					return ImGuiKey_F24;
				case Keyboard::Key::Kp0:
					return ImGuiKey_Keypad0;
				case Keyboard::Key::Kp1:
					return ImGuiKey_Keypad1;
				case Keyboard::Key::Kp2:
					return ImGuiKey_Keypad2;
				case Keyboard::Key::Kp3:
					return ImGuiKey_Keypad3;
				case Keyboard::Key::Kp4:
					return ImGuiKey_Keypad4;
				case Keyboard::Key::Kp5:
					return ImGuiKey_Keypad5;
				case Keyboard::Key::Kp6:
					return ImGuiKey_Keypad6;
				case Keyboard::Key::Kp7:
					return ImGuiKey_Keypad7;
				case Keyboard::Key::Kp8:
					return ImGuiKey_Keypad8;
				case Keyboard::Key::Kp9:
					return ImGuiKey_Keypad9;
				case Keyboard::Key::KpDot:
					return ImGuiKey_KeypadDecimal;
				case Keyboard::Key::KpDivide:
					return ImGuiKey_KeypadDivide;
				case Keyboard::Key::KpMultiply:
					return ImGuiKey_KeypadMultiply;
				case Keyboard::Key::KpSubtract:
					return ImGuiKey_KeypadSubtract;
				case Keyboard::Key::KpAdd:
					return ImGuiKey_KeypadAdd;
				case Keyboard::Key::KpEnter:
					return ImGuiKey_KeypadEnter;
				case Keyboard::Key::KpEqual:
					return ImGuiKey_KeypadEqual;
				case Keyboard::Key::LeftShift:
					return ImGuiKey_LeftShift;
				case Keyboard::Key::LeftControl:
					return ImGuiKey_LeftCtrl;
				case Keyboard::Key::LeftAlt:
					return ImGuiKey_LeftAlt;
				case Keyboard::Key::LeftSuper:
					return ImGuiKey_LeftSuper;
				case Keyboard::Key::RightShift:
					return ImGuiKey_RightShift;
				case Keyboard::Key::RightControl:
					return ImGuiKey_RightCtrl;
				case Keyboard::Key::RightAlt:
					return ImGuiKey_RightAlt;
				case Keyboard::Key::RightSuper:
					return ImGuiKey_RightSuper;
				case Keyboard::Key::Menu:
					return ImGuiKey_Menu;
				default:
					return ImGuiKey_None;
			}
		}

#define IMGUI_EVENT_FUNC_HEADER(return_value)                                                                                    \
	ImGuiContext* context = nullptr;                                                                                             \
	Window* engine_window = window_from(event);                                                                                  \
	{                                                                                                                            \
		ImGuiRenderer::Window* window = nullptr;                                                                                 \
                                                                                                                                 \
		if (engine_window)                                                                                                       \
		{                                                                                                                        \
			window = engine_window->imgui_window();                                                                              \
		}                                                                                                                        \
		else                                                                                                                     \
		{                                                                                                                        \
			return return_value;                                                                                                 \
		}                                                                                                                        \
                                                                                                                                 \
		if (window == nullptr || (context = window->context()) == nullptr)                                                       \
		{                                                                                                                        \
			if (auto render_viewport = engine_window->render_viewport())                                                         \
			{                                                                                                                    \
				if (ImGuiViewportClient* client = Object::instance_cast<ImGuiViewportClient>(render_viewport->client()))         \
				{                                                                                                                \
					context = client->context;                                                                                   \
				}                                                                                                                \
			}                                                                                                                    \
		}                                                                                                                        \
		if (context == nullptr)                                                                                                  \
		{                                                                                                                        \
			return return_value;                                                                                                 \
		}                                                                                                                        \
	}                                                                                                                            \
	ImGuiContextSaver imgui_context_saver(context);

		static void imgui_sent_mouse_position(Engine::Window* engine_window, float x, float y)
		{
			auto& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				auto info = Platform::monitor_info(engine_window->monitor_index());
				auto pos  = engine_window->position();
				io.AddMousePosEvent(x + pos.x, info.size.y - (y + pos.y));
			}
			else
			{
				io.AddMousePosEvent(x, engine_window->cached_size().y - y);
			}
		}

		static void on_mouse_move(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();

			auto& data = event.get<const MouseMotionEvent&>();
			auto& io   = ImGui::GetIO();
			io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);

			imgui_sent_mouse_position(engine_window, data.x, data.y);
		}

		static void on_mouse_button(const Event& event, bool is_pressed)
		{
			IMGUI_EVENT_FUNC_HEADER();

			auto& data		  = event.get<const MouseButtonEvent&>();
			auto& io		  = ImGui::GetIO();
			auto imgui_button = imgui_button_of(data.button);
			if (imgui_button != -1)
			{
				io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
				io.AddMouseButtonEvent(imgui_button, is_pressed);
			}
		}

		static void on_mouse_wheel(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();

			auto& data = event.get<const MouseWheelEvent&>();
			auto& io   = ImGui::GetIO();
			io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
			io.AddMouseWheelEvent(data.x, data.y);
		}

		static void on_keyboard_button(const Event& event, bool is_pressed)
		{
			IMGUI_EVENT_FUNC_HEADER();

			auto& data		  = event.get<const KeyEvent&>();
			auto& io		  = ImGui::GetIO();
			auto imgui_button = imgui_button_of(data.key);

			if (imgui_button != -ImGuiKey_None)
			{
				io.AddKeyEvent(ImGuiMod_Ctrl, ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl));
				io.AddKeyEvent(ImGuiMod_Shift, ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift));
				io.AddKeyEvent(ImGuiMod_Alt, ImGui::IsKeyDown(ImGuiKey_LeftAlt) || ImGui::IsKeyDown(ImGuiKey_RightAlt));
				io.AddKeyEvent(ImGuiMod_Super, ImGui::IsKeyDown(ImGuiKey_LeftSuper) || ImGui::IsKeyDown(ImGuiKey_RightSuper));
				io.AddKeyEvent(imgui_button, is_pressed);
			}
		}

		static void on_text_input(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();
			auto& data = event.get<const TextInputEvent&>();
			auto& io   = ImGui::GetIO();

			io.AddInputCharactersUTF8(data.text.c_str());
		}

		static void on_window_close(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();

			if (auto* vp = ImGui::FindViewportByPlatformHandle(engine_window))
			{
				vp->PlatformRequestClose = true;
			}
		}

		static void on_window_move(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();

			if (auto* vp = ImGui::FindViewportByPlatformHandle(engine_window))
			{
				vp->PlatformRequestMove = true;
			}
		}

		static void on_window_resize(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();

			if (auto* vp = ImGui::FindViewportByPlatformHandle(engine_window))
			{
				vp->PlatformRequestResize = true;
			}
		}

		static void on_finger_down(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();

			const FingerDownEvent& data = event.get<const FingerDownEvent&>();
			if (data.finger_index == 0)
			{
				auto& io = ImGui::GetIO();
				io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
				imgui_sent_mouse_position(engine_window, data.x, data.y);
				io.AddMouseButtonEvent(0, true);
			}
		}

		static void on_finger_up(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();
			const FingerUpEvent& data = event.get<const FingerUpEvent&>();
			if (data.finger_index == 0)
			{
				auto& io = ImGui::GetIO();
				io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
				imgui_sent_mouse_position(engine_window, data.x, data.y);
				io.AddMouseButtonEvent(0, false);
			}
		}

		static void on_finger_motion(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();

			const FingerMotionEvent& data = event.get<const FingerMotionEvent&>();
			if (data.finger_index == 0)
			{
				auto& io = ImGui::GetIO();
				io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
				imgui_sent_mouse_position(engine_window, data.x, data.y);
			}
		}

		ENGINE_EXPORT void on_event_recieved(const Event& event)
		{
			auto type = event.type();

			switch (type)
			{
				case EventType::MouseMotion:
					return on_mouse_move(event);

				case EventType::MouseButtonDown:
				case EventType::MouseButtonUp:
					return on_mouse_button(event, type == EventType::MouseButtonDown);

				case EventType::MouseWheel:
					return on_mouse_wheel(event);

				case EventType::KeyDown:
				case EventType::KeyUp:
					return on_keyboard_button(event, type == EventType::KeyDown);

				case EventType::TextInput:
					return on_text_input(event);

				case EventType::WindowClose:
					return on_window_close(event);
				case EventType::WindowMoved:
					return on_window_move(event);
				case EventType::WindowResized:
					return on_window_resize(event);

				case EventType::FingerDown:
					return on_finger_down(event);
				case EventType::FingerUp:
					return on_finger_up(event);
				case EventType::FingerMotion:
					return on_finger_motion(event);

				default:
					break;
			}
		}

		static EventSystemListenerID m_listener_id;

		ENGINE_EXPORT void disable_events()
		{
			if (!m_listener_id.is_valid())
				return;

			EventSystem* system = EventSystem::instance();
			system->remove_listener(m_listener_id);
			m_listener_id = EventSystemListenerID();
		}

		ENGINE_EXPORT void enable_events()
		{
			if (m_listener_id.is_valid())
				return;

			EventSystem* system = EventSystem::instance();
			m_listener_id		= system->add_listener(EventType::Undefined, on_event_recieved);
		}

		static FORCE_INLINE Window* window_from(ImGuiViewport* vp)
		{
			return reinterpret_cast<Window*>(vp->PlatformHandle);
		}

		static void imgui_trinex_window_create(ImGuiViewport* vp)
		{
			WindowConfig config;
			config.position.x = vp->Pos.x;
			config.position.y = vp->Pos.y;
			config.size.x	  = vp->Size.x;
			config.size.y	  = vp->Size.y;

			config.attributes = {WindowAttribute::Hidden};

			if (vp->Flags & ImGuiViewportFlags_NoDecoration)
			{
				config.attributes.insert(WindowAttribute::BorderLess);
			}
			else
			{
				config.attributes.insert(WindowAttribute::Resizable);
			}

			config.client = "";

			auto parent_window = window_from(ImGui::FindViewportByID(vp->ParentViewportId));
			auto new_window	   = Engine::WindowManager::instance()->create_window(config, parent_window);

			auto render_viewport = new_window->render_viewport();
			auto client			 = Object::new_instance<ImGuiViewportClient>();
			client->viewport	 = vp;
			client->context		 = ImGui::GetCurrentContext();
			render_viewport->client(client);

			vp->PlatformHandle	 = new_window;
			vp->PlatformUserData = reinterpret_cast<void*>(new_window->id());
		}


		static void imgui_trinex_window_destroy(ImGuiViewport* vp)
		{
			if (is_in_logic_thread())
			{
				if (vp->ParentViewportId != 0)
				{
					Identifier id = reinterpret_cast<Identifier>(vp->PlatformUserData);
					if (Window* window = WindowManager::instance()->find(id))
					{
						WindowManager::instance()->destroy_window(window);
					}
				}

				vp->PlatformUserData = vp->PlatformHandle = nullptr;
			}
		}

		static void imgui_trinex_window_show(ImGuiViewport* vp)
		{
			if (Window* wd = window_from(vp))
			{
				wd->show();
			}
		}

		static void imgui_trinex_set_window_pos(ImGuiViewport* vp, ImVec2 pos)
		{
			if (Window* wd = window_from(vp))
			{
				auto info = Platform::monitor_info(wd->monitor_index());
				wd->position({pos.x, info.size.y - (pos.y + wd->cached_size().y)});
			}
		}

		static ImVec2 imgui_trinex_get_window_pos(ImGuiViewport* vp)
		{
			if (Window* wd = window_from(vp))
			{
				auto pos	= wd->position();
				auto info	= Platform::monitor_info(wd->monitor_index());
				float new_y = -pos.y + info.size.y - wd->cached_size().y;
				return {pos.x, new_y};
			}

			return {0, 0};
		}

		static void imgui_trinex_set_window_size(ImGuiViewport* vp, ImVec2 pos)
		{}

		static ImVec2 imgui_trinex_get_window_size(ImGuiViewport* vp)
		{
			if (Window* wd = window_from(vp))
			{
				auto size = wd->size();
				return {size.x, size.y};
			}

			return {0, 0};
		}

		static bool imgui_trinex_get_window_focus(ImGuiViewport* vp)
		{
			if (Window* wd = window_from(vp))
			{
				return wd->focused();
			}
			return false;
		}

		static void imgui_trinex_set_window_focus(ImGuiViewport* vp)
		{
			if (Window* wd = window_from(vp))
			{
				wd->focus();
			}
		}

		static void imgui_trinex_set_window_title(ImGuiViewport* vp, const char* title)
		{
			if (Window* wd = window_from(vp))
			{
				wd->title(title);
			}
		}

		static bool imgui_trinex_get_window_minimized(ImGuiViewport* vp)
		{
			if (Window* wd = window_from(vp))
			{
				return wd->is_iconify();
			}
			return false;
		}

		static void imgui_trinex_window_init_platform_interface(Window* window)
		{
			ImGuiPlatformIO& platform_io			= ImGui::GetPlatformIO();
			platform_io.Platform_CreateWindow		= imgui_trinex_window_create;
			platform_io.Platform_DestroyWindow		= imgui_trinex_window_destroy;
			platform_io.Platform_ShowWindow			= imgui_trinex_window_show;
			platform_io.Platform_SetWindowPos		= imgui_trinex_set_window_pos;
			platform_io.Platform_GetWindowPos		= imgui_trinex_get_window_pos;
			platform_io.Platform_SetWindowSize		= imgui_trinex_set_window_size;
			platform_io.Platform_GetWindowSize		= imgui_trinex_get_window_size;
			platform_io.Platform_SetWindowFocus		= imgui_trinex_set_window_focus;
			platform_io.Platform_GetWindowFocus		= imgui_trinex_get_window_focus;
			platform_io.Platform_GetWindowMinimized = imgui_trinex_get_window_minimized;
			platform_io.Platform_SetWindowTitle		= imgui_trinex_set_window_title;

			ImGuiViewport* main_viewport	= ImGui::GetMainViewport();
			main_viewport->PlatformHandle	= window;
			main_viewport->PlatformUserData = reinterpret_cast<void*>(window->id());
		}

		static void imgui_trinex_window_init(Window* window)
		{
			ImGuiIO& io			   = ImGui::GetIO();
			io.BackendPlatformName = "imgui_impl_android";
			auto bd				   = IM_NEW(ImGuiTrinexWindowData)();

			enable_events();
			io.BackendPlatformUserData = bd;
			io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;


			if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) && (io.BackendFlags & ImGuiBackendFlags_PlatformHasViewports))
				imgui_trinex_window_init_platform_interface(window);
		}

		static void imgui_trinex_window_shutdown_platform_interface(Window* window)
		{
			ImGui::DestroyPlatformWindows();
		}

		static void imgui_trinex_window_shutdown(Window* window)
		{
			imgui_trinex_window_shutdown_platform_interface(window);

			ImGuiIO& io			   = ImGui::GetIO();
			io.BackendPlatformName = nullptr;
			IM_DELETE(reinterpret_cast<ImGuiTrinexWindowData*>(io.BackendPlatformUserData));
			io.BackendPlatformUserData = nullptr;
		}

		static void imgui_trinex_window_update_monitors()
		{
			auto* bd					 = imgui_trinex_backend_data();
			ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
			platform_io.Monitors.resize(0);
			bd->update_monitors = false;

			size_t display_count = Platform::monitors_count();
			for (size_t n = 0; n < display_count; n++)
			{
				ImGuiPlatformMonitor monitor;
				auto info		= Platform::monitor_info(n);
				monitor.WorkPos = monitor.MainPos = ImVec2(info.pos.x, info.pos.y);
				monitor.WorkSize = monitor.MainSize = ImVec2(info.size.x, info.size.y);
				monitor.DpiScale					= info.dpi / 96.0f;

				if (n == 0)
				{
				}

				monitor.PlatformHandle = reinterpret_cast<void*>(n);
				platform_io.Monitors.push_back(monitor);
			}
		}

		static void imgui_trinex_window_new_frame(Window* window)
		{
			ImGuiIO& io = ImGui::GetIO();
			auto bd		= imgui_trinex_backend_data();

			auto size			 = window->cached_size();
			Size2D drawable_size = window->cached_size();

			io.DisplaySize = ImVec2(size.x, size.y);
			if (drawable_size.x > 0 && drawable_size.y > 0)
				io.DisplayFramebufferScale = ImVec2(drawable_size.x / size.x, drawable_size.y / size.y);

			// Update monitors
			if (bd->update_monitors)
				imgui_trinex_window_update_monitors();

			float current_time = engine_instance->time_seconds();
			io.DeltaTime	   = bd->time > 0.0 ? (current_time - bd->time) : (1.0f / 60.0f);
			bd->time		   = current_time;
		}
	}// namespace ImGuiWindowBackend

	Window& Window::imgui_initialize_internal()
	{
		ImGuiWindowBackend::imgui_trinex_window_init(this);
		return *this;
	}

	Window& Window::imgui_terminate_internal()
	{
		ImGuiWindowBackend::imgui_trinex_window_shutdown(this);
		return *this;
	}

	Window& Window::imgui_new_frame()
	{
		ImGuiWindowBackend::imgui_trinex_window_new_frame(this);
		return *this;
	}
}// namespace Engine
