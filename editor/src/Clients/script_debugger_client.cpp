#include <Clients/script_debuger_client.hpp>
#include <Core/base_engine.hpp>
#include <Core/editor_config.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/theme.hpp>
#include <Core/threading.hpp>
#include <Event/event.hpp>
#include <Event/event_data.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/rhi.hpp>
#include <Platform/platform.hpp>
#include <ScriptEngine/script.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <ScriptEngine/script_variable.hpp>
#include <Systems/event_system.hpp>
#include <Window/window.hpp>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace Engine
{
	class DebugExecScriptFunction : public ImGuiWidget
	{
		ImVec2 m_window_pos;
		ScriptFolder* m_root_folder;
		Script* m_script = nullptr;
		ScriptFunction m_function;

	public:
		DebugExecScriptFunction() : m_root_folder(ScriptEngine::scripts_folder())
		{
			auto vp      = ImGui::GetMainViewport();
			m_window_pos = vp->WorkPos + vp->WorkSize * 0.5f;
		}

		void init(RenderViewport* viewport) override
		{}

		void render_script_combo_box(ScriptFolder* folder)
		{
			for (auto& [name, subfolder] : folder->sub_folders())
			{
				render_script_combo_box(subfolder);
			}

			for (auto& [name, script] : folder->scripts())
			{
				if (ImGui::Selectable(script->path().c_str(), script == m_script))
				{
					m_script = script;
					m_function.release();
				}
			}
		}

		void render_function_combo_box()
		{
			if (m_script == nullptr)
				return;

			uint_t count = m_script->functions_count();

			for (uint_t i = 0; i < count; ++i)
			{
				ScriptFunction func = m_script->function_by_index(i);

				if (func.param_count() != 0)
					continue;

				if (ImGui::Selectable(func.name().data()))
				{
					m_function = func;
				}
			}
		}

		bool render(RenderViewport* viewport) override
		{
			bool is_open = true;

			ImGui::SetNextWindowPos(m_window_pos, ImGuiCond_FirstUseEver, {0.5f, 0.5f});
			ImGui::SetNextWindowSize({400, 150}, ImGuiCond_FirstUseEver);

			if (ImGui::Begin("editor/Exec Script Function"_localized, &is_open))
			{
				ImGui::BeginVertical(0, ImGui::GetContentRegionAvail(), 0.f);
				ImGui::Spring(0.f);

				if (ImGui::BeginCombo("editor/Script"_localized, m_script ? m_script->name().c_str() : ""))
				{
					render_script_combo_box(m_root_folder);
					ImGui::EndCombo();
				}
				ImGui::Spring(0.f);

				if (ImGui::BeginCombo("editor/Function"_localized, m_function.is_valid() ? m_function.name().data() : ""))
				{
					render_function_combo_box();
					ImGui::EndCombo();
				}

				ImGui::Spring(1.f);

				ImGui::BeginHorizontal(1);
				{
					ImGui::Spring();

					if (ImGui::Button("editor/Cancel"_localized))
					{
						is_open = false;
					}

					if (ImGui::Button("editor/Execute"_localized))
					{
						if (m_function.is_valid())
						{
							logic_thread()->call([func = m_function]() { ScriptContext::execute(func); });
						}

						is_open = false;
					}

					ImGui::Spring(0.f);

					ImGui::EndHorizontal();
				}

				ImGui::Spring(0.f);
				ImGui::EndVertical();
			}

			ImGui::End();
			return is_open;
		}
	};


	implement_engine_class_default_init(ScriptDebuggerClient, 0);

	ScriptDebuggerClient::ScriptDebuggerClient()
		: m_debugging_thread(new Thread()), m_action(ImGui::TextEditor::DebugAction::Continue), m_is_in_debug_loop(false)
	{
		build_language_definition();
	}

	ScriptDebuggerClient::~ScriptDebuggerClient()
	{
		delete m_debugging_thread;
		m_debugging_thread = nullptr;
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

	ScriptDebuggerClient& ScriptDebuggerClient::build_language_definition()
	{
		m_lang = ImGui::TextEditor::LanguageDefinition::AngelScript();

		for (uint_t module_index = 0, module_count = ScriptEngine::module_count(); module_index < module_count; ++module_index)
		{
			auto module = ScriptEngine::module_by_index(module_index);
			register_module(m_lang, module);
		}

		return *this;
	}

	ScriptDebuggerClient& ScriptDebuggerClient::on_bind_viewport(class RenderViewport* viewport)
	{
		Super::on_bind_viewport(viewport);
		window()->title("Script Debugger");
		ScriptContext::line_callback([this](void*) { on_line_callback(); });
		return *this;
	}

	ScriptDebuggerClient& ScriptDebuggerClient::on_unbind_viewport(class RenderViewport* viewport)
	{
		Super::on_unbind_viewport(viewport);
		ScriptContext::clear_line_callback();
		return *this;
	}

	ScriptDebuggerClient& ScriptDebuggerClient::update(float dt)
	{
		Super::update(dt);

		auto imgui_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(imgui_viewport->WorkPos);
		ImGui::SetNextWindowSize(imgui_viewport->WorkSize);
		ImGui::Begin("Debugger", nullptr,
		             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
		                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar);

		auto dock_id = ImGui::GetID("ScriptDebugger##Dock");
		ImGui::DockSpace(dock_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

		if (imgui_window()->frame_index() == 1)
		{
			ImGui::DockBuilderRemoveNode(dock_id);
			ImGui::DockBuilderAddNode(dock_id,
			                          int_t(ImGuiDockNodeFlags_PassthruCentralNode) | int_t(ImGuiDockNodeFlags_DockSpace));
			ImGui::DockBuilderSetNodeSize(dock_id, ImGui::GetMainViewport()->WorkSize);

			auto dock_id_left                  = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Left, 0.25f, nullptr, &dock_id);
			auto dock_id_variables_and_globals = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Right, 0.25f, nullptr, &dock_id);
			auto dock_id_down                  = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Down, 0.3f, nullptr, &dock_id);

			ImGui::DockBuilderDockWindow("###Bottom", dock_id_down);
			ImGui::DockBuilderDockWindow("###Left", dock_id_left);
			ImGui::DockBuilderDockWindow("###Variables", dock_id_variables_and_globals);
			ImGui::DockBuilderDockWindow("###Globals", dock_id_variables_and_globals);

			ImGui::DockBuilderFinish(dock_id);
		}

		render_bar_menu(dt)
		        .render_left_viewport(dt)
		        .render_code_viewport(dt)
		        .render_variables_viewport(dt)
		        .render_globals_viewport(dt)
		        .render_bottom_viewport(dt);

		ImGui::End();
		return *this;
	}

	ScriptDebuggerClient& ScriptDebuggerClient::render_bar_menu(float dt)
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("editor/File"_localized))
			{
				draw_available_clients_for_opening();
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("editor/View"_localized))
			{
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("editor/Debug"_localized))
			{
				if (ImGui::MenuItem("editor/Exec Function"_localized, "editor/Execute specific function from a module"_localized))
				{
					imgui_window()->widgets_list.create<DebugExecScriptFunction>();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("editor/Build"_localized))
			{
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
		return *this;
	}

	ScriptDebuggerClient& ScriptDebuggerClient::render_code_viewport(float dt)
	{
		for (auto& [name, editor] : m_text_editors)
		{
			if (!editor.m_is_bind_to_dock)
			{
				auto id = ImGui::GetID("ScriptDebugger##Dock");
				ImGui::SetNextWindowDockID(id);
				editor.m_is_bind_to_dock = true;
			}

			if (editor.m_need_focus)
			{
				ImGui::SetNextWindowFocus();
				editor.m_need_focus = false;
			}

			bool is_open = true;

			if (ImGui::Begin(name.c_str(), &is_open, ImGuiWindowFlags_NoCollapse))
			{
				editor.m_editor.Render(name.c_str(), ImGui::GetContentRegionAvail());
			}

			ImGui::End();

			if (is_open == false)
			{
				close_editor(name, editor);
				return *this;
			}
		}

		return *this;
	}

	ScriptDebuggerClient& ScriptDebuggerClient::render_source_tree(class ScriptFolder* folder)
	{
		if (folder == nullptr)
			return *this;

		if (folder == ScriptEngine::scripts_folder() || ImGui::CollapsingHeader(folder->name().c_str()))
		{
			ImGui::Indent();

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

				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					m_script_for_popup = script;
					ImGui::OpenPopup("ScriptPopup");
				}
			}

			ImGui::Unindent();
		}

		return *this;
	}

	ScriptDebuggerClient& ScriptDebuggerClient::render_left_viewport(float dt)
	{
		ImGui::Begin("Source Tree###Left");
		render_source_tree(ScriptEngine::scripts_folder());

		if (ImGui::BeginPopup("ScriptPopup"))
		{
			if (ImGui::Button("editor/Save"_localized))
			{
				auto editor_iterator = m_text_editors.find(m_script_for_popup->path());

				if (editor_iterator != m_text_editors.end())
				{
					auto& editor = editor_iterator->second;

					if (editor.m_editor.IsTextChanged())
					{
						m_script_for_popup->code(editor.m_editor.GetText());
						editor.m_editor.ResetTextChanged();
					}
				}

				if (!m_script_for_popup->save())
				{
					error_log("ScriptDebugger", "Failed to save script '%s'", m_script_for_popup->path().c_str());
				}
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Button("editor/Build"_localized))
			{
				auto editor_iterator = m_text_editors.find(m_script_for_popup->path());

				if (editor_iterator != m_text_editors.end())
				{
					auto& editor = editor_iterator->second;

					if (editor.m_editor.IsTextChanged())
					{
						m_script_for_popup->code(editor.m_editor.GetText());
					}
				}

				m_script_for_popup->build(false);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::End();

		return *this;
	}

	static void draw_variable(StringView ns, StringView name, int_t type_id, byte* address)
	{
		String type = ScriptEngine::type_declaration(type_id, true);
		ScriptTypeInfo info;

		ImGui::TableNextRow();
		bool expand  = ScriptEngine::is_object_type(type_id, true) && address;
		String value = address ? ScriptEngine::to_string(address, type_id) : "null";

		ImGui::TableSetColumnIndex(0);

		if (expand)
		{
			expand        = false;
			bool do_check = true;

			if (ScriptEngine::is_handle_type(type_id) && *reinterpret_cast<byte**>(address) == nullptr)
			{
				do_check = false;
			}

			if (do_check)
			{
				info = ScriptEngine::type_info_by_id(type_id);

				if (info.is_valid())
				{
					expand = info.property_count() > 0;
				}
			}
		}

		if (expand)
		{
			expand = ImGui::TreeNodeEx("###Node", ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_NoAutoOpenOnLog,
			                           "%s%s%s", ns.data(), ns.empty() ? "" : "::", name.data());
		}
		else
		{
			ImGui::Text("%s%s%s", ns.data(), ns.empty() ? "" : "::", name.data());
		}

		ImGui::TableSetColumnIndex(2);
		ImGui::Text("%s", type.c_str());

		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%s", value.c_str());

		if (expand)
		{
			auto prop_count = info.property_count();

			StringView prop_name;
			int_t prop_type_id = 0;
			int_t offset       = 0;
			byte* object_address =
			        ScriptEngine::is_handle_type(type_id) && address ? *reinterpret_cast<byte**>(address) : address;

			ImGui::Indent(25);

			if (object_address)
			{
				for (uint_t i = 0; i < prop_count; ++i)
				{
					if (info.property(i, &prop_name, &prop_type_id, nullptr, nullptr, &offset))
					{
						// if (ScriptEngine::is_object_type(prop_type_id, true))
						// {
						//     draw_variable("", prop_name, prop_type_id, *reinterpret_cast<byte**>(object_address + offset));
						// }
						// else
						{
							draw_variable("", prop_name, prop_type_id, object_address + offset);
						}
					}
				}
			}

			ImGui::Unindent(25);
		}
	}

	ScriptDebuggerClient& ScriptDebuggerClient::render_variables_viewport(float dt)
	{
		bool render = ImGui::Begin("Variables###Variables");

		if (!m_is_in_debug_loop)
		{
			ImGui::End();
			return *this;
		}

		if (render)
		{
			uint_t variables = ScriptContext::var_count(m_current_stack_level);

			ImGui::BeginTable("###Variables", 3, ImGuiTableFlags_Resizable);

			ImGui::TableSetupColumn("Name###Name", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_IndentEnable);
			ImGui::TableSetupColumn("Value###Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Type###Type", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableHeadersRow();

			{
				byte* self    = ScriptContext::this_pointer(m_current_stack_level);
				int_t type_id = ScriptContext::this_type_id(m_current_stack_level);

				if (self && type_id > 0)
				{
					draw_variable("", "this", type_id, self);
				}
			}

			for (uint_t i = 0; i < variables; ++i)
			{
				StringView name;
				int_t type_id;

				if (!ScriptContext::var(i, m_current_stack_level, &name, &type_id) || name.empty())
					continue;

				byte* address = ScriptContext::address_of_var(i, m_current_stack_level);
				draw_variable("", name, type_id, address);
			}

			ImGui::EndTable();
		}

		ImGui::End();
		return *this;
	}

	ScriptDebuggerClient& ScriptDebuggerClient::render_globals_viewport(float dt)
	{
		bool render = ImGui::Begin("Globals###Globals");

		if (!m_is_in_debug_loop)
		{
			ImGui::End();
			return *this;
		}

		if (render)
		{
			ImGui::BeginTable("###Variables", 3, ImGuiTableFlags_Resizable);

			ImGui::TableSetupColumn("Name###Name", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_IndentEnable);
			ImGui::TableSetupColumn("Value###Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Type###Type", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableHeadersRow();

			// Render engine global variables
			uint_t count = ScriptEngine::global_property_count();

			for (uint_t i = 0; i < count; ++i)
			{
				StringView name;
				StringView ns;
				int_t type_id = 0;
				byte* address = nullptr;

				if (ScriptEngine::global_property(i, &name, &ns, nullptr, &type_id, nullptr, &address))
				{
					draw_variable(ns, name, type_id, address);
				}
			}

			count = ScriptEngine::module_count();

			for (uint_t i = 0; i < count; ++i)
			{
				auto module = ScriptEngine::module_by_index(i);

				uint_t count = module.global_var_count();

				for (uint_t i = 0; i < count; ++i)
				{
					StringView name;
					StringView ns;
					int_t type_id = 0;

					if (module.global_var(i, &name, &ns, &type_id))
					{
						if (void* address = module.address_of_global_var(i))
						{
							draw_variable(ns, name, type_id, reinterpret_cast<byte*>(address));
						}
					}
				}
			}

			ImGui::EndTable();
		}

		ImGui::End();
		return *this;
	}

	ScriptDebuggerClient& ScriptDebuggerClient::render_bottom_viewport(float dt)
	{
		ImGui::Begin("Call Stack###Bottom");

		if (m_is_in_debug_loop)
		{
			auto callstack_size = ScriptContext::callstack_size();

			ImGui::BeginTable("##Callstack", 5, ImGuiTableFlags_Resizable);

			ImGui::TableSetupColumn("Index###Index", ImGuiTableColumnFlags_WidthStretch, 0.1f);
			ImGui::TableSetupColumn("Function###Name", ImGuiTableColumnFlags_WidthStretch, 0.26f);
			ImGui::TableSetupColumn("File###File", ImGuiTableColumnFlags_WidthStretch, 0.26f);
			ImGui::TableSetupColumn("Line###Line", ImGuiTableColumnFlags_WidthStretch, 0.1f);
			ImGui::TableSetupColumn("Address###Address", ImGuiTableColumnFlags_WidthStretch, 0.26f);
			ImGui::TableHeadersRow();

			for (uint_t i = 0, func_index = 0; i < callstack_size; ++i)
			{
				auto func = ScriptContext::function(i);
				auto line = ScriptContext::line_position(i).y;

				if (line <= 0)
					continue;

				++func_index;

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("%u", func_index);

				ImGui::TableNextColumn();

				String name = String(func.namespace_name());
				if (!name.empty())
					name += "::";
				name += func.name();

				ImGui::Selectable(name.c_str(), i == m_current_stack_level, ImGuiSelectableFlags_SpanAllColumns);

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (m_current_editor)
					{
						m_current_editor->m_editor.SetCurrentLineIndicator(-1, false);
					}

					open_script(func);
					m_current_editor->m_editor.SetCurrentLineIndicator(line, false);
					m_current_stack_level = i;
				}

				ImGui::TableNextColumn();
				if (Script* script = func.module().script())
				{
					ImGui::Text("%s", script->path().c_str());
				}
				else
				{
					ImGui::Text("Undefined");
				}

				ImGui::TableNextColumn();
				ImGui::Text("%d", line);

				ImGui::TableNextColumn();
				ImGui::Text("%p", func.function());
			}

			ImGui::EndTable();
		}

		ImGui::End();
		return *this;
	}

	void ScriptDebuggerClient::on_debugger_jump(ImGui::TextEditor* editor, int_t line)
	{}

	void ScriptDebuggerClient::on_debugger_action(ImGui::TextEditor* editor, ImGui::TextEditor::DebugAction action)
	{
		if (!m_is_in_debug_loop)
			return;

		m_action = action;

		switch (action)
		{
			case ImGui::TextEditor::DebugAction::Step:
			case ImGui::TextEditor::DebugAction::StepOut:
				m_last_command_at_stack_level = ScriptContext::callstack_size();

			case ImGui::TextEditor::DebugAction::StepInto:
			case ImGui::TextEditor::DebugAction::Continue:
				m_is_in_debug_loop = false;
				break;

			default:
				break;
		}
	}

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
	{}

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
		int_t line = ScriptContext::line_position().y;

		if (line <= 0)
			return *this;

		auto func = ScriptContext::function();

		if (line == m_last_line && func == m_last_function)
			return *this;

		m_last_function = func;
		m_last_line     = line;

		if (m_current_editor)
			m_current_editor->m_editor.SetCurrentLineIndicator(-1, false);

		if (ScriptContext::state() != ScriptContext::State::Active || engine_instance->is_requesting_exit())
			return *this;

		if (m_action == ImGui::TextEditor::DebugAction::Continue)
		{
			if (!check_breakpoints())
				return *this;
		}
		else if (m_action == ImGui::TextEditor::DebugAction::Step)
		{
			if (ScriptContext::callstack_size() > m_last_command_at_stack_level)
			{
				if (!check_breakpoints())
					return *this;
			}
		}
		else if (m_action == ImGui::TextEditor::DebugAction::StepOut)
		{
			if (ScriptContext::callstack_size() >= m_last_command_at_stack_level)
			{
				if (!check_breakpoints())
					return *this;
			}
		}

		open_script(func);
		if (m_current_editor)
			m_current_editor->m_editor.SetCurrentLineIndicator(line, false);
		return lock_logic_thread();
	}

	ScriptDebuggerClient& ScriptDebuggerClient::open_script(Script* script)
	{
		if (m_current_editor && m_current_editor->m_script == script)
			return *this;

		String path  = script->path().str();
		auto& editor = m_text_editors[path];

		if (!editor.m_script)
		{
			editor.m_editor.OnDebuggerJump =
			        std::bind(&This::on_debugger_jump, this, std::placeholders::_1, std::placeholders::_2);
			editor.m_editor.OnDebuggerAction =
			        std::bind(&This::on_debugger_action, this, std::placeholders::_1, std::placeholders::_2);
			editor.m_editor.OnIdentifierHover =
			        std::bind(&This::on_identifier_hover, this, std::placeholders::_1, std::placeholders::_2);
			editor.m_editor.HasIdentifierHover =
			        std::bind(&This::has_identifier_hover, this, std::placeholders::_1, std::placeholders::_2);
			editor.m_editor.OnExpressionHover =
			        std::bind(&This::on_expression_hover, this, std::placeholders::_1, std::placeholders::_2);
			editor.m_editor.HasExpressionHover =
			        std::bind(&This::has_expression_hover, this, std::placeholders::_1, std::placeholders::_2);
			editor.m_editor.OnBreakpointRemove =
			        std::bind(&This::on_breakpoint_remove, this, std::placeholders::_1, std::placeholders::_2);
			editor.m_editor.OnBreakpointUpdate =
			        std::bind(&This::on_breakpoint_update, this, std::placeholders::_1, std::placeholders::_2);
			editor.m_editor.OnCtrlAltClick = std::bind(&This::on_ctrl_alt_click, this, std::placeholders::_1,
			                                           std::placeholders::_2, std::placeholders::_3);
			editor.m_editor.RequestOpen =
			        std::bind(&This::on_request_open, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			editor.m_editor.OnContentUpdate = std::bind(&This::on_content_update, this, std::placeholders::_1);

			editor.m_script = script;
			editor.m_editor.SetLanguageDefinition(m_lang);
			editor.m_editor.SetText(script->code());
		}

		editor.m_need_focus = true;
		m_current_editor    = &editor;
		return *this;
	}

	ScriptDebuggerClient& ScriptDebuggerClient::open_script(const Path& path)
	{
		if (m_current_editor && m_current_editor->m_script->path() == path)
		{
			m_current_editor->m_need_focus = true;
			return *this;
		}

		return open_script(ScriptEngine::scripts_folder()->find_script(path, true));
	}

	ScriptDebuggerClient& ScriptDebuggerClient::open_script(const ScriptFunction& function)
	{
		Script* script = function.module().script();
		if (script)
			return open_script(script);
		return *this;
	}

	ScriptDebuggerClient& ScriptDebuggerClient::close_editor(const String& path, Editor& editor)
	{
		m_text_editors.erase(path);
		return *this;
	}

	void ScriptDebuggerClient::on_event_recieved(const Event& event, void* userdata)
	{
		ScriptDebuggerClient* self = reinterpret_cast<ScriptDebuggerClient*>(userdata);

		if (event.type() == EventType::Quit ||
		    (event.type() == EventType::WindowClose && event.window_id() == self->window()->id()))
		{
			self->m_is_in_debug_loop = false;
		}

		if (!self->m_is_in_debug_loop)
		{
			self->m_recieved_events.push_back(event);
			return;
		}

		bool push_to_recieved_events = true;
		ImGuiBackend_Window::on_event_recieved(event);

		if (event.type() == EventType::KeyUp)
		{
			auto& data = event.get<const KeyUpEvent&>();

			switch (data.key)
			{
				case Engine::Keyboard::Key::F5:
					self->on_debugger_action(nullptr, ImGui::TextEditor::DebugAction::Continue);
					break;
				case Engine::Keyboard::Key::F10:
					self->on_debugger_action(nullptr, ImGui::TextEditor::DebugAction::Step);
					break;
				case Engine::Keyboard::Key::F11:
					self->on_debugger_action(nullptr, ImGui::TextEditor::DebugAction::StepInto);
					break;

				default:
					break;
			}
		}
		else if (event.type() == EventType::WindowResized && self->window()->id() == event.window_id())
		{
			const WindowEvent& window_event = event.get<const WindowEvent&>();
			auto x                          = window_event.x;
			auto y                          = window_event.y;
			self->viewport()->on_resize({x, y});

			push_to_recieved_events = false;
		}

		if (push_to_recieved_events)
			self->m_recieved_events.push_back(event);
	}

	ScriptDebuggerClient& ScriptDebuggerClient::debugger_thread_loop()
	{
		m_is_in_debug_loop    = true;
		m_current_stack_level = 0;

		auto prev_time = engine_instance->time_seconds() - 0.033f;

		while (m_is_in_debug_loop)
		{
			float current_time = engine_instance->time_seconds();
			float dt           = current_time - prev_time;
			prev_time          = current_time;


			Platform::WindowManager::pool_events(on_event_recieved, this);

			if (m_is_in_debug_loop)
			{
				viewport()->update(dt);

				m_render_finished.lock();
				viewport()->render();
				render_thread()->call([]() { rhi->submit(); });
			}
		}

		return *this;
	}

	ScriptDebuggerClient& ScriptDebuggerClient::lock_logic_thread()
	{
		if (is_in_logic_thread())
		{
			struct RunDebugLoop : public Task<RunDebugLoop> {
				ScriptDebuggerClient* m_client;
				ScriptDebuggerClient& (ScriptDebuggerClient::*m_method)();
				RunDebugLoop(ScriptDebuggerClient* client, ScriptDebuggerClient& (ScriptDebuggerClient::*method)())
				    : m_client(client), m_method(method)
				{}

				void execute() override
				{
					(m_client->*m_method)();
				}
			};

			m_debugging_thread->create_task<RunDebugLoop>(this, &This::debugger_thread_loop);
			m_debugging_thread->wait();

			if (!m_recieved_events.empty())
			{
				ImGuiBackend_Window::disable_events();
			}

			for (auto& event : m_recieved_events)
			{
				EventSystem::instance()->push_event(event);
			}

			if (!m_recieved_events.empty())
			{
				ImGuiBackend_Window::enable_events();
			}

			m_recieved_events.clear();
		}

		return *this;
	}

	bool ScriptDebuggerClient::check_breakpoints()
	{
		auto line = ScriptContext::line_position().y;
		auto func = ScriptContext::function();

		Script* script = func.module().script();
		if (script == nullptr)
			return false;

		auto editor = m_text_editors.find(script->path().str());
		if (editor == m_text_editors.end())
			return false;

		for (auto& breakpoint : editor->second.m_editor.GetBreakpoints())
		{
			if (breakpoint.mLine == line)
			{
				return true;
			}
		}

		return false;
	}

	ScriptDebuggerClient& ScriptDebuggerClient::on_script_select(class Script* script, bool setup_for_edit)
	{
		m_selected_script = script;

		if (setup_for_edit)
		{
			open_script(script);
		}
		return *this;
	}
}// namespace Engine
