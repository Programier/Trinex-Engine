#include <Core/pointer.hpp>
#include <Graphics/render_viewport.hpp>
#include <TextEditor.h>
#include <debugger.h>

#include <ScriptEngine/script_function.hpp>

namespace Engine
{
    namespace ImGuiRenderer
    {
        class Window;
    }

    class ScriptDebuggerClient : public ViewportClient
    {
        declare_class(ScriptDebuggerClient, ViewportClient);

        enum class DebugAction
        {
            Continue,
            StepInto,
            StepOver,
            StepOut
        };

        ImGui::TextEditor m_text_editor;

        Pointer<RenderViewport> m_viewport;
        class Window* m_window                      = nullptr;
        class ImGuiRenderer::Window* m_imgui_window = nullptr;

        class Script* m_selected_script = nullptr;
        class Script* m_editing_script  = nullptr;

        ScriptFunction m_last_function;
        DebugAction m_action;
        uint_t m_last_command_at_stack_level;

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

        ScriptDebuggerClient& build_language_definiiton();
        ScriptDebuggerClient& on_line_callback();
        bool check_breakpoints();

    public:
        ScriptDebuggerClient();
        ScriptDebuggerClient& on_bind_viewport(class RenderViewport* viewport) override;
        ScriptDebuggerClient& on_unbind_viewport(class RenderViewport* viewport) override;
        ScriptDebuggerClient& render(class RenderViewport* viewport) override;
        ScriptDebuggerClient& update(class RenderViewport* viewport, float dt) override;

        ScriptDebuggerClient& render_code_viewport(float dt);
        ScriptDebuggerClient& render_source_tree(class ScriptFolder* folder);
        ScriptDebuggerClient& render_left_viewport(float dt);
        ScriptDebuggerClient& render_right_viewport(float dt);
        ScriptDebuggerClient& render_bottom_viewport(float dt);

        ScriptDebuggerClient& on_script_select(class Script* script, bool setup_for_edit);
        ScriptDebuggerClient& add_breakpoint(uint32_t line_index);
        ScriptDebuggerClient& remove_breakpoint(uint32_t line_index);
    };
}// namespace Engine
