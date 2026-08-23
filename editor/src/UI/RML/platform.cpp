#include <Core/base_engine.hpp>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/SystemInterface.h>
#include <imgui.h>

#include <algorithm>
#include <unordered_map>

namespace Trinex::UI::RML::Backend
{
	namespace
	{
		struct ContextState {
			bool mouse_inside                       = false;
			bool mouse_down[5]                      = {};
			bool keys_down[ImGuiKey_NamedKey_COUNT] = {};
		};

		static std::unordered_map<Rml::Context*, ContextState> s_context_states;

		static ContextState& context_state(Rml::Context* context)
		{
			return s_context_states[context];
		}

		static int mouse_button_index(ImGuiMouseButton button)
		{
			switch (button)
			{
				case ImGuiMouseButton_Left: return 0;
				case ImGuiMouseButton_Right: return 1;
				case ImGuiMouseButton_Middle: return 2;
				default: return -1;
			}
		}

		static ImGuiMouseCursor mouse_cursor_from_rml(const Rml::String& cursor_name)
		{
			if (cursor_name == "pointer")
				return ImGuiMouseCursor_Hand;
			if (cursor_name == "text")
				return ImGuiMouseCursor_TextInput;
			if (cursor_name == "move")
				return ImGuiMouseCursor_ResizeAll;
			if (cursor_name == "resize")
				return ImGuiMouseCursor_ResizeAll;
			if (cursor_name == "cross")
				return ImGuiMouseCursor_Arrow;
			if (cursor_name == "unavailable")
				return ImGuiMouseCursor_NotAllowed;
			if (cursor_name == "resize-horizontal")
				return ImGuiMouseCursor_ResizeEW;
			if (cursor_name == "resize-vertical")
				return ImGuiMouseCursor_ResizeNS;
			if (cursor_name == "resize-diagonal-tl-br")
				return ImGuiMouseCursor_ResizeNWSE;
			if (cursor_name == "resize-diagonal-tr-bl")
				return ImGuiMouseCursor_ResizeNESW;
			return ImGuiMouseCursor_Arrow;
		}

		static Rml::Input::KeyIdentifier key_identifier_from_imgui(ImGuiKey key)
		{
			using namespace Rml::Input;

			if (key >= ImGuiKey_0 && key <= ImGuiKey_9)
				return static_cast<KeyIdentifier>(KI_0 + (key - ImGuiKey_0));
			if (key >= ImGuiKey_A && key <= ImGuiKey_Z)
				return static_cast<KeyIdentifier>(KI_A + (key - ImGuiKey_A));
			if (key >= ImGuiKey_F1 && key <= ImGuiKey_F24)
				return static_cast<KeyIdentifier>(KI_F1 + (key - ImGuiKey_F1));
			if (key >= ImGuiKey_Keypad0 && key <= ImGuiKey_Keypad9)
				return static_cast<KeyIdentifier>(KI_NUMPAD0 + (key - ImGuiKey_Keypad0));

			switch (key)
			{
				case ImGuiKey_Tab: return KI_TAB;
				case ImGuiKey_LeftArrow: return KI_LEFT;
				case ImGuiKey_RightArrow: return KI_RIGHT;
				case ImGuiKey_UpArrow: return KI_UP;
				case ImGuiKey_DownArrow: return KI_DOWN;
				case ImGuiKey_PageUp: return KI_PRIOR;
				case ImGuiKey_PageDown: return KI_NEXT;
				case ImGuiKey_Home: return KI_HOME;
				case ImGuiKey_End: return KI_END;
				case ImGuiKey_Insert: return KI_INSERT;
				case ImGuiKey_Delete: return KI_DELETE;
				case ImGuiKey_Backspace: return KI_BACK;
				case ImGuiKey_Space: return KI_SPACE;
				case ImGuiKey_Enter: return KI_RETURN;
				case ImGuiKey_Escape: return KI_ESCAPE;
				case ImGuiKey_LeftCtrl: return KI_LCONTROL;
				case ImGuiKey_LeftShift: return KI_LSHIFT;
				case ImGuiKey_LeftAlt: return KI_LMENU;
				case ImGuiKey_LeftSuper: return KI_LMETA;
				case ImGuiKey_RightCtrl: return KI_RCONTROL;
				case ImGuiKey_RightShift: return KI_RSHIFT;
				case ImGuiKey_RightAlt: return KI_RMENU;
				case ImGuiKey_RightSuper: return KI_RMETA;
				case ImGuiKey_Menu: return KI_APPS;
				case ImGuiKey_CapsLock: return KI_CAPITAL;
				case ImGuiKey_ScrollLock: return KI_SCROLL;
				case ImGuiKey_NumLock: return KI_NUMLOCK;
				case ImGuiKey_PrintScreen: return KI_SNAPSHOT;
				case ImGuiKey_Pause: return KI_PAUSE;
				case ImGuiKey_Apostrophe: return KI_OEM_7;
				case ImGuiKey_Comma: return KI_OEM_COMMA;
				case ImGuiKey_Minus: return KI_OEM_MINUS;
				case ImGuiKey_Period: return KI_OEM_PERIOD;
				case ImGuiKey_Slash: return KI_OEM_2;
				case ImGuiKey_Semicolon: return KI_OEM_1;
				case ImGuiKey_Equal: return KI_OEM_PLUS;
				case ImGuiKey_LeftBracket: return KI_OEM_4;
				case ImGuiKey_Backslash: return KI_OEM_5;
				case ImGuiKey_RightBracket: return KI_OEM_6;
				case ImGuiKey_GraveAccent: return KI_OEM_3;
				case ImGuiKey_KeypadDecimal: return KI_DECIMAL;
				case ImGuiKey_KeypadDivide: return KI_DIVIDE;
				case ImGuiKey_KeypadMultiply: return KI_MULTIPLY;
				case ImGuiKey_KeypadSubtract: return KI_SUBTRACT;
				case ImGuiKey_KeypadAdd: return KI_ADD;
				case ImGuiKey_KeypadEnter: return KI_NUMPADENTER;
				case ImGuiKey_KeypadEqual: return KI_OEM_NEC_EQUAL;
				default: return KI_UNKNOWN;
			}
		}

		class RMLSystemInterface final : public Rml::SystemInterface
		{
		public:
			double GetElapsedTime() override { return engine_instance ? engine_instance->time_seconds() : ImGui::GetTime(); }

			void SetMouseCursor(const Rml::String& cursor_name) override
			{
				if (ImGui::GetCurrentContext())
					ImGui::SetMouseCursor(mouse_cursor_from_rml(cursor_name));
			}

			void SetClipboardText(const Rml::String& text) override
			{
				if (ImGui::GetCurrentContext())
					ImGui::SetClipboardText(text.c_str());
			}

			void GetClipboardText(Rml::String& text) override
			{
				text.clear();

				if (ImGui::GetCurrentContext())
				{
					if (const char* clipboard = ImGui::GetClipboardText())
						text = clipboard;
				}
			}
		};
	}// namespace

	trinex_on_pre_init()
	{
		static RMLSystemInterface interface;
		Rml::SetSystemInterface(&interface);
	}

	trinex_on_post_shutdown()
	{
		Rml::SetSystemInterface(nullptr);
	}

	int key_modifier_state()
	{
		int state = 0;

		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))
			state |= Rml::Input::KM_CTRL;
		if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))
			state |= Rml::Input::KM_SHIFT;
		if (ImGui::IsKeyDown(ImGuiKey_LeftAlt) || ImGui::IsKeyDown(ImGuiKey_RightAlt))
			state |= Rml::Input::KM_ALT;
		if (ImGui::IsKeyDown(ImGuiKey_LeftSuper) || ImGui::IsKeyDown(ImGuiKey_RightSuper))
			state |= Rml::Input::KM_META;

		return state;
	}

	Rml::Rectanglei desktop_rect()
	{
		Rml::Vector2i min;
		Rml::Vector2i max;
		bool valid = false;

		for (const ImGuiPlatformMonitor& monitor : ImGui::GetPlatformIO().Monitors)
		{
			const Rml::Vector2i pos  = {static_cast<int>(monitor.MainPos.x), static_cast<int>(monitor.MainPos.y)};
			const Rml::Vector2i size = {static_cast<int>(monitor.MainSize.x), static_cast<int>(monitor.MainSize.y)};

			if (!valid)
			{
				min   = pos;
				max   = pos + size;
				valid = true;
			}
			else
			{
				min.x = std::min(min.x, pos.x);
				min.y = std::min(min.y, pos.y);
				max.x = std::max(max.x, pos.x + size.x);
				max.y = std::max(max.y, pos.y + size.y);
			}
		}

		if (!valid)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			if (viewport)
			{
				min = {static_cast<int>(viewport->Pos.x), static_cast<int>(viewport->Pos.y)};
				max = min + Rml::Vector2i(static_cast<int>(viewport->Size.x), static_cast<int>(viewport->Size.y));
			}
			else
			{
				const ImVec2 size = ImGui::GetIO().DisplaySize;
				max               = {static_cast<int>(size.x), static_cast<int>(size.y)};
			}
		}

		return Rml::Rectanglei::FromPositionSize(min, max - min);
	}

	void update_context_dimensions(Rml::Context* context)
	{
		if (context == nullptr)
			return;

		const Rml::Vector2i size = desktop_rect().Size();
		if (context->GetDimensions() != size)
			context->SetDimensions(size);
	}

	void forget_context(Rml::Context* context)
	{
		s_context_states.erase(context);
	}

	bool process_imgui_events(Rml::Context* context, ImVec2 viewport_origin, ImVec2 viewport_size)
	{
		if (context == nullptr || !ImGui::GetCurrentContext())
			return true;

		const Rml::Vector2i context_origin = desktop_rect().TopLeft();
		ContextState& state                = context_state(context);
		const ImGuiIO& io                  = ImGui::GetIO();
		const int modifiers                = key_modifier_state();
		bool not_consumed                  = true;

		const ImVec2 mouse_pos  = ImGui::GetMousePos();
		const bool mouse_inside = mouse_pos.x >= viewport_origin.x && mouse_pos.y >= viewport_origin.y &&
		                          mouse_pos.x < viewport_origin.x + viewport_size.x &&
		                          mouse_pos.y < viewport_origin.y + viewport_size.y;

		if (mouse_inside)
		{
			const int x = static_cast<int>(mouse_pos.x) - context_origin.x;
			const int y = static_cast<int>(mouse_pos.y) - context_origin.y;
			not_consumed &= context->ProcessMouseMove(x, y, modifiers);
		}
		else if (state.mouse_inside)
		{
			not_consumed &= context->ProcessMouseLeave();
		}
		state.mouse_inside = mouse_inside;

		if (mouse_inside)
		{
			for (ImGuiMouseButton button = 0; button < ImGuiMouseButton_COUNT; ++button)
			{
				const int rml_button = mouse_button_index(button);
				if (rml_button < 0)
					continue;

				const bool down = ImGui::IsMouseDown(button);
				if (down == state.mouse_down[button])
					continue;

				state.mouse_down[button] = down;
				not_consumed &= down ? context->ProcessMouseButtonDown(rml_button, modifiers)
				                     : context->ProcessMouseButtonUp(rml_button, modifiers);
			}

			if (io.MouseWheel != 0.f || io.MouseWheelH != 0.f)
				not_consumed &= context->ProcessMouseWheel(Rml::Vector2f(io.MouseWheelH, io.MouseWheel), modifiers);
		}

		for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = static_cast<ImGuiKey>(key + 1))
		{
			const Rml::Input::KeyIdentifier key_identifier = key_identifier_from_imgui(key);
			if (key_identifier == Rml::Input::KI_UNKNOWN)
				continue;

			const int index = key - ImGuiKey_NamedKey_BEGIN;
			const bool down = ImGui::IsKeyDown(key);
			if (down == state.keys_down[index])
				continue;

			state.keys_down[index] = down;
			not_consumed &=
			        down ? context->ProcessKeyDown(key_identifier, modifiers) : context->ProcessKeyUp(key_identifier, modifiers);
		}

		for (ImWchar character : io.InputQueueCharacters)
		{
			if (character != 0)
				not_consumed &= context->ProcessTextInput(static_cast<Rml::Character>(character));
		}

		return not_consumed;
	}
}// namespace Trinex::UI::RML::Backend
