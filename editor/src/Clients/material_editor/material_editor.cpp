#include <Clients/material_editor_client.hpp>
#include <Core/base_engine.hpp>
#include <Core/blueprints.hpp>
#include <Core/editor_config.hpp>
#include <Core/group.hpp>
#include <Core/localization.hpp>
#include <Core/reflection/class.hpp>
#include <Core/shader_compiler.hpp>
#include <Core/theme.hpp>
#include <Core/threading.hpp>
#include <Engine/settings.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture_2D.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_graph.hpp>
#include <Widgets/content_browser.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Widgets/property_renderer.hpp>
#include <Window/window.hpp>
#include <imgui_internal.h>
#include <imgui_node_editor.h>
#include <imgui_stacklayout.h>

#define MATERIAL_EDITOR_DEBUG 1

namespace Engine
{
	implement_engine_class(MaterialEditorClient, 0)
	{
		register_client(Material::static_class_instance(), static_class_instance());
	}


	class ImGuiMaterialPreview : public ImGuiWidget
	{
	public:
		void init(RenderViewport* viewport)
		{}

		bool render(RenderViewport* viewport)
		{
			bool is_open = true;
			ImGui::Begin(name(), &is_open);
			ImGui::End();
			return is_open;
		}

		static const char* name()
		{
			return static_name();
		}

		static const char* static_name()
		{
			return "editor/MaterialPreview"_localized;
		}
	};


	class ImGuiMaterialCode : public ImGuiWidget
	{
	public:
		String code;

		void init(RenderViewport* viewport)
		{}

		bool render(RenderViewport* viewport)
		{
			bool is_open = true;
			ImGui::SetNextWindowSize({300, 400}, ImGuiCond_Appearing);
			ImGui::Begin(name(), &is_open);
			ImGui::InputTextMultiline("##Code", code.data(), code.size(), ImGui::GetContentRegionAvail(),
			                          ImGuiInputTextFlags_ReadOnly);
			ImGui::End();
			return is_open;
		}

		virtual const char* name() const
		{
			return static_name();
		}

		static const char* static_name()
		{
			return "editor/Material Code"_localized;
		}
	};

	class ImGuiNodeProperties : public PropertyRenderer
	{
	public:
		const char* name() const override
		{
			return static_name();
		}

		static const char* static_name()
		{
			return "editor/Node Properties"_localized;
		}
	};

	MaterialEditorClient::MaterialEditorClient()
	{
		ax::NodeEditor::Config config;
		config.SettingsFile    = nullptr;
		m_graph_editor_context = ax::NodeEditor::CreateEditor(&config);
	}

	MaterialEditorClient::~MaterialEditorClient()
	{
		ax::NodeEditor::DestroyEditor(m_graph_editor_context);
	}

	MaterialEditorClient& MaterialEditorClient::create_content_browser()
	{
		m_content_browser = imgui_window()->widgets_list.create<ContentBrowser>();
		m_content_browser->on_close.push([this]() { m_content_browser = nullptr; });
		return *this;
	}

	MaterialEditorClient& MaterialEditorClient::create_preview_window()
	{
		m_preview_window = imgui_window()->widgets_list.create<ImGuiMaterialPreview>();
		m_preview_window->on_close.push([this]() { m_preview_window = nullptr; });
		return *this;
	}

	MaterialEditorClient& MaterialEditorClient::create_material_code_window()
	{
		m_material_code = imgui_window()->widgets_list.create<ImGuiMaterialCode>();
		m_material_code->on_close.push([this]() { m_material_code = nullptr; });
		return *this;
	}

	MaterialEditorClient& MaterialEditorClient::create_properties_window()
	{
		m_properties_window = imgui_window()->widgets_list.create<PropertyRenderer>();
		m_properties_window->on_close.push([this]() { m_properties_window = nullptr; });
		return *this;
	}

	MaterialEditorClient& MaterialEditorClient::create_node_properties_window()
	{
		m_node_properties_window = imgui_window()->widgets_list.create<ImGuiNodeProperties>();
		m_node_properties_window->on_close.push([this]() { m_node_properties_window = nullptr; });
		return *this;
	}

	MaterialEditorClient& MaterialEditorClient::on_bind_viewport(class RenderViewport* viewport)
	{
		Super::on_bind_viewport(viewport);

		auto wd = window();

		String new_title = Strings::format("Trinex Material Editor [{} RHI]", rhi->info.name.c_str());
		wd->title(new_title);

		render_thread()->wait();

		ImGuiWindow* prev_window = ImGuiWindow::current();
		ImGuiWindow::make_current(imgui_window());

		create_content_browser().create_preview_window().create_properties_window().create_node_properties_window();

		ImGuiWindow::make_current(prev_window);
		m_compiler = ShaderCompiler::Compiler::static_create_compiler();
		return *this;
	}

	MaterialEditorClient& MaterialEditorClient::select(Object* object)
	{
		Super::select(object);

		if (m_material == object)
			return *this;

		if (Material* material = object->instance_cast<Material>())
		{
			m_material = material;

			if (m_properties_window)
			{
				m_properties_window->update(material);
			}

			on_node_select(nullptr);
		}

		return *this;
	}

	void MaterialEditorClient::on_node_select(Object* object)
	{
		if (m_selected_node == object)
			return;

		m_selected_node = Object::instance_cast<VisualMaterialGraph::Node>(object);

		if (m_node_properties_window)
			m_node_properties_window->update(m_selected_node);
	}

	void MaterialEditorClient::render_dock_window()
	{
		auto dock_id                       = ImGui::GetID("MaterialEditorDock##Dock");
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
		ImGui::DockSpace(dock_id, ImVec2(0.0f, 0.0f), dockspace_flags);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("editor/View"_localized))
			{
				draw_available_clients_for_opening();

				if (ImGui::MenuItem("editor/Open Content Browser"_localized, nullptr, false, m_content_browser == nullptr))
				{
					create_content_browser();
				}

				if (ImGui::MenuItem("editor/Open Material Code"_localized, nullptr, nullptr, m_material_code == nullptr))
				{
					create_material_code_window();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("editor/Edit"_localized))
			{
				if (ImGui::MenuItem("editor/Reload localization"_localized))
				{
					Localization::instance()->reload();
				}

				if (ImGui::BeginMenu("editor/Change language"_localized))
				{
					for (const String& lang : Settings::languages)
					{
						const char* localized = Object::localize("editor/" + lang).c_str();
						if (ImGui::MenuItem(localized))
						{
							Object::language(lang);
							break;
						}
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("editor/Material"_localized))
			{
				if (ImGui::MenuItem("Compile source", nullptr, false, m_material != nullptr && m_compiler != 0))
				{
					m_material->compile(m_compiler);
				}

				if (ImGui::MenuItem("Just apply", nullptr, false, m_material != nullptr))
				{
					m_material->apply_changes();
				}

				if (ImGui::MenuItem("Update Source", nullptr, false, m_material != nullptr && m_material_code != nullptr))
				{
					m_material->shader_source(m_material_code->code);
				}
				ImGui::EndMenu();
			}


			ImGui::EndMenuBar();
		}

		if (ImGui::IsWindowAppearing())
		{
			ImGui::DockBuilderRemoveNode(dock_id);
			ImGui::DockBuilderAddNode(dock_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dock_id, ImGui::GetMainViewport()->WorkSize);


			auto dock_id_down      = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Down, 0.3f, nullptr, &dock_id);
			auto dock_id_left      = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Left, 0.2f, nullptr, &dock_id);
			auto dock_id_left_up   = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Up, 0.5f, nullptr, &dock_id_left);
			auto dock_id_left_down = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.5f, nullptr, &dock_id_left);
			auto dock_id_right     = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Right, 0.2f, nullptr, &dock_id);

			ImGui::DockBuilderDockWindow(ContentBrowser::static_name(), dock_id_down);
			ImGui::DockBuilderDockWindow(ImGuiMaterialPreview::static_name(), dock_id_left_up);
			ImGui::DockBuilderDockWindow(ImGuiNodeProperties::static_name(), dock_id_left_down);
			ImGui::DockBuilderDockWindow(PropertyRenderer::static_name(), dock_id_right);
			ImGui::DockBuilderDockWindow("###Material Source", dock_id);

			ImGui::DockBuilderFinish(dock_id);
		}
	}

	MaterialEditorClient& MaterialEditorClient::update(float dt)
	{
		Super::update(dt);

		ImGuiViewport* imgui_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(imgui_viewport->WorkPos);
		ImGui::SetNextWindowSize(imgui_viewport->WorkSize);

		ImGui::Begin("MaterialEditorDock", nullptr,
		             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
		                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus |
		                     ImGuiWindowFlags_MenuBar);
		render_dock_window();

		render_viewport(dt);

		ImGui::End();
		return *this;
	}

	MaterialEditorClient& MaterialEditorClient::render_viewport(float dt)
	{
		ImGui::Begin("editor/Material Source###Material Source"_localized);
		render_visual_material_graph(Object::instance_cast<VisualMaterial>(m_material));
		ImGui::End();

		//ImGui::SetNextWindowSize({500, 200}, ImGuiCond_Appearing);
		// ImGui::Begin("Graph Source Code");
		// ImGui::InputTextMultiline("##Text", m_material_source.data(), m_material_source.size(), ImGui::GetContentRegionAvail(),
		//                           ImGuiInputTextFlags_ReadOnly);
		// ImGui::End();
		return *this;
	}

	void MaterialEditorClient::on_object_dropped(Object* object)
	{}

	MaterialEditorClient& MaterialEditorClient::update_drag_and_drop()
	{
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ContentBrowser->Object");
			if (payload)
			{
				IM_ASSERT(payload->DataSize == sizeof(Object*));
				on_object_dropped(*reinterpret_cast<Object**>(payload->Data));
			}
			ImGui::EndDragDropTarget();
		}
		return *this;
	}
	static InitializeController controller([]() { Object::load_object("Example::TestMaterial"); });
}// namespace Engine
