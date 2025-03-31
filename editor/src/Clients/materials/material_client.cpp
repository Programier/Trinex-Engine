#include <Clients/material/material_client.hpp>
#include <Core/localization.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <Engine/settings.hpp>
#include <Graphics/material.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader_compiler.hpp>
#include <Widgets/content_browser.hpp>
#include <Widgets/property_renderer.hpp>
#include <Window/window.hpp>
#include <imgui_internal.h>

namespace Engine
{
	trinex_implement_engine_class(MaterialEditorClient, 0)
	{
		register_client(Material::static_class_instance(), static_class_instance());
	}

	class ImGuiMaterialPreview : public ImGuiWidget
	{
	public:
		void init(RenderViewport* viewport) {}

		bool render(RenderViewport* viewport)
		{
			bool is_open = true;
			ImGui::Begin(name(), &is_open);
			ImGui::End();
			return is_open;
		}

		static const char* name() { return static_name(); }

		static const char* static_name() { return "editor/MaterialPreview"_localized; }
	};

	class ImGuiMaterialCode : public ImGuiWidget
	{
	public:
		String code;

		void init(RenderViewport* viewport) {}

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

		virtual const char* name() const { return static_name(); }

		static const char* static_name() { return "editor/Material Code"_localized; }
	};

	MaterialEditorClient::MaterialEditorClient()
	{
		menu_bar.create("editor/View")->actions.push([this]() {
			draw_available_clients_for_opening();

			if (ImGui::BeginMenu("editor/Material Editor"))
			{
				if (ImGui::MenuItem("editor/Open Content Browser"_localized, nullptr, false, m_content_browser == nullptr))
				{
					create_content_browser();
				}

				ImGui::EndMenu();
			}
		});

		menu_bar.create("editor/Edit")->actions.push([]() {
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
		});

		menu_bar.create("editor/Material")->actions.push([this]() {
			if (ImGui::MenuItem("Compile source", nullptr, false, m_material != nullptr))
			{
				if (auto compiler = ShaderCompiler::instance())
				{
					compiler->compile(m_material);
				}
				else
				{
					error_log("MaterialEditor", "Failed to get material compiler");
				}
			}

			if (ImGui::MenuItem("Just apply", nullptr, false, m_material != nullptr))
			{
				m_material->apply_changes();
			}
		});
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

	MaterialEditorClient& MaterialEditorClient::create_properties_window()
	{
		m_properties_window = imgui_window()->widgets_list.create<PropertyRenderer>();
		m_properties_window->on_close.push([this]() { m_properties_window = nullptr; });
		return *this;
	}

	MaterialEditorClient& MaterialEditorClient::on_bind_viewport(class RenderViewport* viewport)
	{
		Super::on_bind_viewport(viewport);

		auto wd = window();
		wd->title(Strings::format("Trinex Material Editor [{} RHI]", rhi->info.name.c_str()));

		ImGuiWindow* prev_window = ImGuiWindow::current();

		ImGuiWindow::make_current(imgui_window());
		create_content_browser().create_preview_window().create_properties_window();
		ImGuiWindow::make_current(prev_window);
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
				m_properties_window->object(material);
			}
		}

		return *this;
	}

	MaterialEditorClient& MaterialEditorClient::update(float dt)
	{
		Super::update(dt);
		return *this;
	}

	uint32_t MaterialEditorClient::build_dock(uint32_t dock)
	{
		auto dock_id_down  = ImGui::DockBuilderSplitNode(dock, ImGuiDir_Down, 0.3f, nullptr, &dock);
		auto dock_id_left  = ImGui::DockBuilderSplitNode(dock, ImGuiDir_Left, 0.2f, nullptr, &dock);
		auto dock_id_right = ImGui::DockBuilderSplitNode(dock, ImGuiDir_Right, 0.2f, nullptr, &dock);

		ImGui::DockBuilderDockWindow(ContentBrowser::static_name(), dock_id_down);
		ImGui::DockBuilderDockWindow(ImGuiMaterialPreview::static_name(), dock_id_left);
		ImGui::DockBuilderDockWindow(PropertyRenderer::static_name(), dock_id_right);
		ImGui::DockBuilderDockWindow("###Material Source", dock);
		return dock;
	}
}// namespace Engine
