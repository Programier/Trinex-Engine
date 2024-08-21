#include <Core/pointer.hpp>
#include <Core/thread.hpp>
#include <Graphics/render_viewport.hpp>
#include <ScriptEngine/script_function.hpp>
#include <TextEditor.h>
#include <debugger.h>

namespace Engine
{
	namespace ImGuiRenderer
	{
		class Window;
	}

	class ScriptDebuggerClient : public ViewportClient
	{
		declare_class(ScriptDebuggerClient, ViewportClient);

		struct Editor {
			ImGui::TextEditor m_editor;
			class Script* m_script = nullptr;
			bool m_is_bind_to_dock = false;
			bool m_need_focus      = false;
		};

		ImGui::TextEditor::LanguageDefinition m_lang;
		TreeMap<String, Editor> m_text_editors;
		Editor* m_current_editor = nullptr;

		Pointer<RenderViewport> m_viewport;
		Vector<class Event> m_recieved_events;
		class Window* m_window                      = nullptr;
		class ImGuiRenderer::Window* m_imgui_window = nullptr;
		Thread* m_debugging_thread;

		class Script* m_selected_script  = nullptr;
		class Script* m_script_for_popup = nullptr;

		ScriptFunction m_last_function;
		int_t m_last_line = -1;

		ImGui::TextEditor::DebugAction m_action;
		uint_t m_last_command_at_stack_level;
		uint_t m_current_stack_level;

		bool m_is_in_debug_loop;

		void on_debugger_jump(ImGui::TextEditor* editor, int_t line);
		void on_debugger_action(ImGui::TextEditor* editor, ImGui::TextEditor::DebugAction);
		void on_identifier_hover(ImGui::TextEditor* editor, const String& identifier);
		bool has_identifier_hover(ImGui::TextEditor* editor, const String& identifier);
		void on_expression_hover(ImGui::TextEditor* editor, const String& identifier);
		bool has_expression_hover(ImGui::TextEditor* editor, const String& identifier);
		void on_breakpoint_remove(ImGui::TextEditor* editor, int_t line);
		void on_breakpoint_update(ImGui::TextEditor* editor, int_t line);
		void on_ctrl_alt_click(ImGui::TextEditor* editor, const String&, ImGui::TextEditor::Coordinates coords);
		void on_request_open(ImGui::TextEditor* editor, const std::string&, const std::string&);
		void on_content_update(ImGui::TextEditor* editor);
		static void on_event_recieved(const class Event& event, void* userdata);

		ScriptDebuggerClient& build_language_definition();
		ScriptDebuggerClient& on_line_callback();
		ScriptDebuggerClient& debugger_thread_loop();
		ScriptDebuggerClient& lock_logic_thread();
		ScriptDebuggerClient& open_script(const ScriptFunction& function);
		ScriptDebuggerClient& close_editor(const String& path, Editor& editor);

		bool check_breakpoints();

	public:
		ScriptDebuggerClient();
		~ScriptDebuggerClient();
		ScriptDebuggerClient& on_bind_viewport(class RenderViewport* viewport) override;
		ScriptDebuggerClient& on_unbind_viewport(class RenderViewport* viewport) override;
		ScriptDebuggerClient& render(class RenderViewport* viewport) override;
		ScriptDebuggerClient& update(class RenderViewport* viewport, float dt) override;

		ScriptDebuggerClient& open_script(Script* script);
		ScriptDebuggerClient& open_script(const Path& path);

		ScriptDebuggerClient& render_bar_menu(float dt);
		ScriptDebuggerClient& render_code_viewport(float dt);
		ScriptDebuggerClient& render_source_tree(class ScriptFolder* folder);
		ScriptDebuggerClient& render_left_viewport(float dt);
		ScriptDebuggerClient& render_variables_viewport(float dt);
		ScriptDebuggerClient& render_globals_viewport(float dt);
		ScriptDebuggerClient& render_bottom_viewport(float dt);

		ScriptDebuggerClient& on_script_select(class Script* script, bool setup_for_edit);
	};
}// namespace Engine
