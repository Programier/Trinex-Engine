#include <Clients/script_debuger.hpp>
#include <Core/class.hpp>
#include <Core/logger.hpp>
#include <Graphics/imgui.hpp>
#include <ScriptEngine/script.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <Window/window.hpp>
#include <editor_config.hpp>
#include <imgui_internal.h>
#include <theme.hpp>

namespace Engine
{
    implement_engine_class_default_init(ScriptDebuggerClient, 0);

    ScriptDebuggerClient::ScriptDebuggerClient() : m_action(DebugAction::Continue)
    {
        build_language_definiiton();

        m_text_editor.OnDebuggerJump   = std::bind(&This::on_debugger_jump, this, std::placeholders::_1, std::placeholders::_2);
        m_text_editor.OnDebuggerAction = std::bind(&This::on_debugger_action, this, std::placeholders::_1, std::placeholders::_2);
        m_text_editor.OnIdentifierHover =
                std::bind(&This::on_identifier_hover, this, std::placeholders::_1, std::placeholders::_2);
        m_text_editor.HasIdentifierHover =
                std::bind(&This::has_identifier_hover, this, std::placeholders::_1, std::placeholders::_2);
        m_text_editor.OnExpressionHover =
                std::bind(&This::on_expression_hover, this, std::placeholders::_1, std::placeholders::_2);
        m_text_editor.HasExpressionHover =
                std::bind(&This::has_expression_hover, this, std::placeholders::_1, std::placeholders::_2);
        m_text_editor.OnBreakpointRemove =
                std::bind(&This::on_breakpoint_remove, this, std::placeholders::_1, std::placeholders::_2);
        m_text_editor.OnBreakpointUpdate =
                std::bind(&This::on_breakpoint_update, this, std::placeholders::_1, std::placeholders::_2);
        m_text_editor.OnCtrlAltClick =
                std::bind(&This::on_ctrl_alt_click, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        m_text_editor.RequestOpen =
                std::bind(&This::on_request_open, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        m_text_editor.OnContentUpdate = std::bind(&This::on_content_update, this, std::placeholders::_1);
        ScriptContext::line_callback([this](void*) { on_line_callback(); });
    }

    static FORCE_INLINE void register_module(ImGui::TextEditor::LanguageDefinition& lang, ScriptModule& module)
    {
        auto count = module.object_type_count();

        for (uint_t i = 0; i < count; i++)
        {
            auto type = module.object_type_by_index(i);

            ImGui::TextEditor::Identifier id;
            id.mDeclaration = type.name();
            lang.mIdentifiers.insert(std::make_pair(id.mDeclaration, id));
        }

        count = module.functions_count();

        for (uint_t i = 0; i < count; i++)
        {
            auto func = module.function_by_index(i);

            ImGui::TextEditor::Identifier id;
            id.mDeclaration = func.name();
            lang.mIdentifiers.insert(std::make_pair(id.mDeclaration, id));
        }
    }

    ScriptDebuggerClient& ScriptDebuggerClient::build_language_definiiton()
    {
        ImGui::TextEditor::LanguageDefinition m_lang = ImGui::TextEditor::LanguageDefinition::AngelScript();

        for (uint_t module_index = 0, module_count = ScriptEngine::module_count(); module_index < module_count; ++module_index)
        {
            auto module = ScriptEngine::module_by_index(module_index);
            register_module(m_lang, module);
        }

        m_text_editor.SetLanguageDefinition(m_lang);
        return *this;
    }

    ScriptDebuggerClient& ScriptDebuggerClient::on_bind_viewport(class RenderViewport* viewport)
    {
        m_window = viewport->window();
        if (m_window == nullptr)
            return *this;
        m_viewport = viewport;

        m_window->imgui_initialize(initialize_theme);
        m_imgui_window = m_window->imgui_window();

        m_window->title("Script Debugger");

        m_imgui_window->reset_frame_index();
        return *this;
    }

    ScriptDebuggerClient& ScriptDebuggerClient::on_unbind_viewport(class RenderViewport* viewport)
    {
        m_window   = nullptr;
        m_viewport = nullptr;
        return *this;
    }

    ScriptDebuggerClient& ScriptDebuggerClient::render(class RenderViewport* viewport)
    {
        viewport->rhi_bind();
        viewport->rhi_clear_color(Color(0, 0, 0, 1.f));
        m_imgui_window->rhi_render();
        return *this;
    }

    ScriptDebuggerClient& ScriptDebuggerClient::update(class RenderViewport* viewport, float dt)
    {
        m_imgui_window->new_frame();

        auto imgui_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(imgui_viewport->WorkPos);
        ImGui::SetNextWindowSize(imgui_viewport->WorkSize);
        ImGui::Begin("Debugger", nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoTitleBar);

        auto dock_id                       = ImGui::GetID("ScriptDebugger##Dock");
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpace(dock_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        if (m_imgui_window->frame_index() == 1)
        {
            ImGui::DockBuilderRemoveNode(dock_id);
            ImGui::DockBuilderAddNode(dock_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dock_id, ImGui::GetMainViewport()->WorkSize);

            auto dock_id_left  = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Left, 0.25f, nullptr, &dock_id);
            auto dock_id_right = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Right, 0.25f, nullptr, &dock_id);
            auto dock_id_down  = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Down, 0.3f, nullptr, &dock_id);

            ImGui::DockBuilderDockWindow("###Bottom", dock_id_down);
            ImGui::DockBuilderDockWindow("###Left", dock_id_left);
            ImGui::DockBuilderDockWindow("###Right", dock_id_right);
            ImGui::DockBuilderDockWindow("###Code", dock_id);

            ImGui::DockBuilderFinish(dock_id);
        }

        render_left_viewport(dt).render_code_viewport(dt).render_right_viewport(dt).render_bottom_viewport(dt);

        ImGui::End();
        m_imgui_window->end_frame();
        return *this;
    }

    ScriptDebuggerClient& ScriptDebuggerClient::render_code_viewport(float dt)
    {
        ImGui::Begin("Code###Code");
        m_text_editor.Render("Debugger", ImGui::GetContentRegionAvail());
        ImGui::End();
        return *this;
    }

    ScriptDebuggerClient& ScriptDebuggerClient::render_source_tree(class ScriptFolder* folder)
    {
        if (folder == nullptr)
            return *this;

        ImGui::PushID(folder);
        if (folder == ScriptEngine::scripts_folder() || ImGui::CollapsingHeader(folder->name().c_str()))
        {
            ImGui::Indent(Settings::ed_collapsing_indent);

            for (auto [name, sub_folder] : folder->sub_folders())
            {
                render_source_tree(sub_folder);
            }

            for (auto [name, script] : folder->scripts())
            {
                if (ImGui::Selectable(name.c_str()))
                {
                    on_script_select(script, false);
                }

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                {
                    on_script_select(script, true);
                }
            }

            ImGui::Unindent(Settings::ed_collapsing_indent);
        }

        ImGui::PopID();
        return *this;
    }

    ScriptDebuggerClient& ScriptDebuggerClient::render_left_viewport(float dt)
    {
        ImGui::Begin("Source Tree###Left");
        render_source_tree(ScriptEngine::scripts_folder());
        ImGui::End();
        return *this;
    }

    ScriptDebuggerClient& ScriptDebuggerClient::render_right_viewport(float dt)
    {
        ImGui::Begin("Variables###Right");
        ImGui::End();
        return *this;
    }

    ScriptDebuggerClient& ScriptDebuggerClient::render_bottom_viewport(float dt)
    {
        ImGui::Begin("Call Stack###Bottom");
        ImGui::End();
        return *this;
    }

    void ScriptDebuggerClient::on_debugger_jump(ImGui::TextEditor* editor, int_t line)
    {}

    void ScriptDebuggerClient::on_debugger_action(ImGui::TextEditor* editor, ImGui::TextEditor::DebugAction)
    {}

    void ScriptDebuggerClient::on_identifier_hover(ImGui::TextEditor* editor, const String& identifier)
    {}

    bool ScriptDebuggerClient::has_identifier_hover(ImGui::TextEditor* editor, const String& identifier)
    {
        return false;
    }

    void ScriptDebuggerClient::on_expression_hover(ImGui::TextEditor* editor, const String& identifier)
    {}

    bool ScriptDebuggerClient::has_expression_hover(ImGui::TextEditor* editor, const String& identifier)
    {
        return false;
    }

    void ScriptDebuggerClient::on_breakpoint_remove(ImGui::TextEditor* editor, int_t line)
    {
        remove_breakpoint(line);
    }

    void ScriptDebuggerClient::on_breakpoint_update(ImGui::TextEditor* editor, int_t line)
    {}

    void ScriptDebuggerClient::on_ctrl_alt_click(ImGui::TextEditor* editor, const String&, ImGui::TextEditor::Coordinates coords)
    {}

    void ScriptDebuggerClient::on_request_open(ImGui::TextEditor* editor, const String&, const String&)
    {}

    void ScriptDebuggerClient::on_content_update(ImGui::TextEditor* editor)
    {}

    ScriptDebuggerClient& ScriptDebuggerClient::on_line_callback()
    {
        if (ScriptContext::state() != ScriptContext::State::Active)
            return *this;

        if (m_action == DebugAction::Continue)
        {
            if (!check_breakpoints())
                return *this;
        }
        else if (m_action == DebugAction::StepOver)
        {
            if (ScriptContext::callstack_size() > m_last_command_at_stack_level)
            {
                if (!check_breakpoints())
                    return *this;
            }
        }
        else if (m_action == DebugAction::StepOut)
        {
            if (ScriptContext::callstack_size() > m_last_command_at_stack_level)
            {
                if (!check_breakpoints())
                    return *this;
            }
        }
        else if (m_action == DebugAction::StepInto)
        {
            check_breakpoints();
        }

        return *this;
    }

    bool ScriptDebuggerClient::check_breakpoints()
    {
        auto& breakpoints = m_text_editor.GetBreakpoints();
        auto line           = ScriptContext::line_position().y;
        m_last_function = ScriptContext::function();

        for (auto& breakpoint : breakpoints)
        {
            if (breakpoint.mLine == line)
            {
                return ScriptContext::suspend();
            }
        }

        return false;
    }

    ScriptDebuggerClient& ScriptDebuggerClient::on_script_select(class Script* script, bool setup_for_edit)
    {
        m_selected_script = script;

        if (setup_for_edit)
        {
            if (m_text_editor.IsTextChanged())
            {
                m_editing_script->code(m_text_editor.GetText());
            }

            m_text_editor.SetText(m_selected_script ? m_selected_script->code() : "");
            m_editing_script = script;
        }
        return *this;
    }

    ScriptDebuggerClient& ScriptDebuggerClient::add_breakpoint(uint32_t line_index)
    {
        m_text_editor.SetBreakpointEnabled(line_index, true);
        return *this;
    }

    ScriptDebuggerClient& ScriptDebuggerClient::remove_breakpoint(uint32_t line_index)
    {
        m_text_editor.SetBreakpointEnabled(line_index, false);
        return *this;
    }

}// namespace Engine
