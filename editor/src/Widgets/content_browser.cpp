#include <Clients/imgui_client.hpp>
#include <Core/constants.hpp>
#include <Core/filesystem/directory_iterator.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/icons.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <Core/theme.hpp>
#include <Engine/project.hpp>
#include <Graphics/texture_2D.hpp>
#include <Widgets/content_browser.hpp>
#include <Widgets/imgui_windows.hpp>
#include <imgui_internal.h>

namespace Engine
{
	void ContentBrowser::init(RenderViewport* viewport)
	{
		m_selected_package = Object::root_package();
	}

	bool ContentBrowser::render_package_popup(void* data)
	{
		bool is_editable = (m_show_popup_for && m_show_popup_for->is_editable() && m_show_popup_for->is_serializable());


		if (ImGui::Button("editor/Create Package"_localized))
		{
			ImGuiWindow::current()->widgets_list.create_identified<ImGuiCreateNewPackage>(this, m_show_popup_for);
			return false;
		}

		if (is_editable && ImGui::Button("editor/Rename"_localized))
		{
			ImGuiWindow::current()->widgets_list.create_identified<ImGuiRenameObject>("RenameObject", m_selected_package);
			return false;
		}

		if (is_editable && ImGui::Button("editor/Save"_localized))
		{
			m_show_popup_for->save();
			return false;
		}

		return true;
	}

	void ContentBrowser::render_package_popup()
	{
		if (ImGui::BeginPopup("###PackagePopup"))
		{
			bool is_editable = (m_show_popup_for && m_show_popup_for->is_editable() && m_show_popup_for->is_serializable());

			if (ImGui::Button("editor/Create Package"_localized))
			{
				ImGuiWindow::current()->widgets_list.create_identified<ImGuiCreateNewPackage>(this, m_show_popup_for);
				ImGui::CloseCurrentPopup();
			}

			if (is_editable && ImGui::Button("editor/Rename"_localized))
			{
				ImGuiWindow::current()->widgets_list.create_identified<ImGuiRenameObject>("RenameObject", m_selected_package);
				ImGui::CloseCurrentPopup();
			}

			if (is_editable && ImGui::Button("editor/Save"_localized))
			{
				m_show_popup_for->save();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void ContentBrowser::render_package_tree(Package* node)
	{
		if (node == m_selected_package)
		{
			ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered));
		}

		bool opened          = false;
		bool is_root_package = node == Object::root_package();

		opened = ImGui::TreeNodeEx(node, ImGuiTreeNodeFlags_CollapsingHeader, "%s", node->string_name().c_str());

		if (node == m_selected_package)
		{
			ImGui::PopStyleColor();
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			m_selected_package = node;
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			m_selected_package = node;
			m_show_popup_for   = node;
			ImGui::OpenPopup("###PackagePopup");
		}

		if (opened)
		{
			if (!is_root_package)
				ImGui::Indent(10.f);

			for (auto child : node->objects())
			{
				if (Package* pkg = child->instance_cast<Package>())
				{
					render_package_tree(pkg);
				}
			}

			if (!is_root_package)
				ImGui::Unindent(10.f);
		}
	}

	void ContentBrowser::render_packages()
	{
		ImGui::Begin("##ContentBrowserPackages"_localized, nullptr, ImGuiWindowFlags_NoTitleBar);

		auto icon      = Icons::icon(Icons::IconType::Add);
		auto icon_size = ImGui::GetFontSize();

		if (ImGui::ImageButton(icon, {icon_size, icon_size}))
		{
			Flags<ImGuiOpenFile::Flag> flags = Flags(ImGuiOpenFile::MultipleSelection);
			auto window                      = ImGuiWindow::current()->widgets_list.create_identified<ImGuiOpenFile>(this, flags);
			window->on_select.push([](const Path& path) {
				Path relative = path.relative(Project::assets_dir);
				Object::load_object_from_file(relative);
			});
			window->type_filters({Constants::asset_extention});
			window->pwd(Project::assets_dir);
		}

		ImGui::SameLine();
		render_package_tree(Object::root_package());
		render_package_popup();
		ImGui::End();
	}

	bool ContentBrowser::render_content_item(Object* object, const StringView& name, const ImVec2& item_size,
	                                         const ImVec2& content_size, bool& not_first_item)
	{
		float padding = ImGui::GetStyle().FramePadding.x;

		bool in_filter = filters.empty();

		if (!in_filter)
		{
			for (auto* callback : filters.callbacks())
			{
				in_filter = (*callback)(object->class_instance());
				if (in_filter)
					break;
			}
		}

		if (!in_filter)
			return false;


		if (not_first_item)
		{
			ImGui::SameLine();

			if (ImGui::GetCursorPosX() + item_size.x >= content_size.x)
			{
				ImGui::NewLine();
				ImGui::NewLine();
			}
		}
		else
		{
			not_first_item = true;
		}

		ImGui::BeginGroup();

		Texture2D* imgui_texture = Icons::find_imgui_icon(object);

		if (imgui_texture)
		{
			ImGui::PushID(name.data());

			bool is_selected = selected_object == object;

			if (is_selected)
			{
				static ImVec4 color1 = ImVec4(79.f / 255.f, 109.f / 255.f, 231.f / 255.f, 1.0),
				              color2 = ImVec4(114.f / 255.f, 138.f / 255.f, 233.f / 255.f, 1.0);

				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(color1));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(color2));
			}

			ImVec2 item_start = ImGui::GetCursorPos();

			bool is_single_press = ImGui::ImageButton(imgui_texture, item_size);
			bool is_double_press = false;
			bool is_item_hovered = ImGui::IsItemHovered();

			if (is_item_hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				is_single_press = false;
				is_double_press = true;
			}

			if (is_single_press)
			{
				selected_object = object;
				on_object_select(object);
			}
			else if (is_double_press)
			{
				if (Package* new_package = object->instance_cast<Package>())
				{
					m_selected_package = new_package;
				}
				else
				{
					if (auto client = ImGuiEditorClient::client_of(object->class_instance(), true))
					{
						client->select(object);
					}
				}

				on_object_double_click(object);
			}
			else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("ContentBrowser->Object", &object, sizeof(Object**));
				ImVec2 item_start = ImGui::GetCursorPos();
				ImGui::Image(imgui_texture, item_size);

				if (imgui_texture == Icons::default_texture())
				{
					const char* class_name = object->class_instance()->display_name().c_str();
					ImVec2 text_size       = ImGui::CalcTextSize(class_name, nullptr, false, item_size.x);

					ImVec2 text_pos = item_start + ImVec2(((item_size.x / 2) - (text_size.x / 2)) + padding,
					                                      (item_size.y / 2) - (text_size.y / 2));

					ImGui::SetCursorPos(text_pos);
					ImGui::PushTextWrapPos(text_pos.x + item_size.x);

					ImGui::TextWrapped("%s", class_name);
					ImGui::PopTextWrapPos();
				}

				ImGui::EndDragDropSource();
			}

			if (is_selected)
			{
				ImGui::PopStyleColor(2);
			}

			ImVec2 current_pos = ImGui::GetCursorPos();

			if (imgui_texture == Icons::default_texture())
			{
				const char* class_name = object->class_instance()->display_name().c_str();
				ImVec2 text_size       = ImGui::CalcTextSize(class_name, nullptr, false, item_size.x);

				ImVec2 text_pos = item_start + ImVec2(((item_size.x / 2) - (text_size.x / 2)) + padding,
				                                      (item_size.y / 2) - (text_size.y / 2));

				ImGui::SetCursorPos(text_pos);
				ImGui::PushTextWrapPos(text_pos.x + item_size.x);

				ImGui::TextWrapped("%s", class_name);
				ImGui::PopTextWrapPos();
			}


			String object_name = Strings::make_sentence(String(name));
			float offset       = (item_size.x - ImGui::CalcTextSize(object_name.c_str(), nullptr, false, item_size.x).x) / 2.f;
			current_pos.x += offset;
			ImGui::SetCursorPos(current_pos);

			ImGui::PushTextWrapPos(current_pos.x + item_size.x - offset);
			ImGui::TextWrapped("%s", object_name.c_str());
			ImGui::PopTextWrapPos();

			ImGui::PopID();
		}
		else
		{
			if (ImGui::Selectable(name.data(), selected_object == object, 0, item_size))
			{
				selected_object = object;
				on_object_select(object);
			}
		}

		ImGui::EndGroup();

		return true;
	}


	static Package* render_package_path(Package* package)
	{
		if (package == nullptr)
			return nullptr;

		Package* result = render_package_path(package->package());
		ImGui::Text("/");
		bool is_pressed = ImGui::SmallButton(package->name().c_str());
		return is_pressed ? package : result;
	}

	void ContentBrowser::render_content_window()
	{
		const ImVec2 item_size = ImVec2(5, 5) * ImGui::GetFontSize();

		ImGui::Begin("##ContentBrowserItems", nullptr, ImGuiWindowFlags_MenuBar);

		ImGui::BeginMenuBar();
		if (Package* new_package = render_package_path(m_selected_package))
		{
			m_selected_package = new_package;
		}
		ImGui::EndMenuBar();

		Package* package = m_selected_package;

		if (package == nullptr)
		{
			ImGui::Text("%s!", "editor/No package selected"_localized);
			ImGui::End();
			return;
		}

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("###ContentContextMenu");
		}

		ImVec2 content_size = ImGui::GetContentRegionAvail();
		bool not_first_item = false;

		size_t rendered = 0;

		for (auto& object : package->objects())
		{
			if (render_content_item(object, object->name(), item_size, content_size, not_first_item))
			{
				++rendered;
			}
		}

		if (rendered == 0)
		{
			ImGui::Text("%s", "editor/No objects found"_localized);
		}

		if (ImGui::BeginPopup("###ContentContextMenu"))
		{
			Package* pkg = selected_package();

			if (ImGui::Button("editor/Create new asset"_localized))
			{
				ImGuiWindow::current()->widgets_list.create<ImGuiCreateNewAsset>(pkg, filters);
				ImGui::CloseCurrentPopup();
			}

			bool is_editable_object = selected_object && selected_object->is_editable();

			if (is_editable_object && ImGui::Button("editor/Rename"_localized))
			{
				ImGuiWindow::current()->widgets_list.create<ImGuiRenameObject>(selected_object);
				ImGui::CloseCurrentPopup();
			}

			if (is_editable_object && ImGui::Button("editor/Delete"_localized))
			{
				Package* package = selected_object->package();
				package->remove_object(selected_object);
				GarbageCollector::destroy(selected_object);
				selected_object = nullptr;
				on_object_select(nullptr);
				ImGui::CloseCurrentPopup();
			}

			if (is_editable_object && ImGui::Button("editor/Save"_localized))
			{
				selected_object->save();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void ContentBrowser::create_dock_space()
	{
		m_dock_window_id = ImGui::GetID("##ContentBrowserDockSpace");

		ImGui::DockSpace(m_dock_window_id, {0, 0},
		                 ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoUndocking | ImGuiDockNodeFlags_NoTabBar);

		if (frame_number == 1)
		{
			ImGui::DockBuilderRemoveNode(m_dock_window_id);
			ImGui::DockBuilderAddNode(m_dock_window_id, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(m_dock_window_id, ImGui::GetWindowSize());

			auto dock_id_left = ImGui::DockBuilderSplitNode(m_dock_window_id, ImGuiDir_Left, 0.2f, nullptr, &m_dock_window_id);

			ImGui::DockBuilderDockWindow("##ContentBrowserPackages", dock_id_left);
			ImGui::DockBuilderDockWindow("##ContentBrowserItems", m_dock_window_id);
			ImGui::DockBuilderFinish(m_dock_window_id);
		}
	}

	bool ContentBrowser::render(RenderViewport* viewport)
	{
		bool open = true;
		ImGui::Begin(name(), closable ? &open : nullptr);
		create_dock_space();

		render_packages();
		render_content_window();

		ImGui::End();
		return open;
	}

	Package* ContentBrowser::selected_package() const
	{
		return m_selected_package;
	}

	const char* ContentBrowser::name() const
	{
		return static_name();
	}

	const char* ContentBrowser::static_name()
	{
		return "editor/Content Browser Title"_localized;
	}

	ContentBrowser::~ContentBrowser()
	{}
}// namespace Engine
