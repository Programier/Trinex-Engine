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
#include <imgui_stacklayout.h>

namespace Engine
{
	void ContentBrowser::init(RenderViewport* viewport)
	{
		m_selected_package = Object::root_package();
	}

	void ContentBrowser::selecte_new_object(Object* object)
	{
		selected_object = object;
		on_object_select(object);
		m_is_renaming = false;
	}

	void ContentBrowser::begin_renaming(Object* object)
	{
		if (object == nullptr)
		{
			object = selected_object;
		}

		if (!object->is_editable())
			return;

		m_is_renaming     = true;
		selected_object   = object;
		m_new_object_name = selected_object->name();
	}

	bool ContentBrowser::render_package_popup(void* data)
	{
		bool is_editable = (m_show_popup_for && m_show_popup_for->is_editable() && m_show_popup_for->is_serializable());


		if (ImGui::Button("editor/Create Package"_localized))
		{
			ImGuiWindow::current()->widgets.create_identified<ImGuiCreateNewPackage>(this, m_show_popup_for);
			return false;
		}

		if (is_editable && ImGui::Button("editor/Rename"_localized))
		{
			ImGuiWindow::current()->widgets.create_identified<ImGuiRenameObject>("RenameObject", m_selected_package);
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
				ImGuiWindow::current()->widgets.create_identified<ImGuiCreateNewPackage>(this, m_show_popup_for);
				ImGui::CloseCurrentPopup();
			}

			if (is_editable && ImGui::Button("editor/Rename"_localized))
			{
				ImGuiWindow::current()->widgets.create_identified<ImGuiRenameObject>("RenameObject", m_selected_package);
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
			auto active_color = ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive);
			ImGui::PushStyleColor(ImGuiCol_Header, active_color);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::MakeHoveredColor(active_color));
		}

		bool opened          = false;
		bool is_root_package = node == Object::root_package();

		opened = ImGui::TreeNodeEx(node, ImGuiTreeNodeFlags_CollapsingHeader, "%s", node->string_name().c_str());

		if (node == m_selected_package)
		{
			ImGui::PopStyleColor(2);
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
			auto window                      = ImGuiWindow::current()->widgets.create_identified<ImGuiOpenFile>(this, flags);
			window->on_select.push([](const Path& path) {
				Path relative = path.relative(rootfs()->native_path(Project::assets_dir));
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

	bool ContentBrowser::render_content_item(Object* object, const ImVec2& item_size)
	{
		bool in_filter  = filters.empty();
		StringView name = object->name();

		if (!in_filter)
		{
			for (auto& callback : filters)
			{
				in_filter = callback(object->class_instance());
				if (in_filter)
					break;
			}
		}

		if (!in_filter)
			return false;

		ImTextureID imgui_texture     = Icons::find_icon(object);
		const float image_side_length = item_size.x * 0.93f;
		const ImVec2 image_size       = ImVec2(image_side_length, image_side_length);

		const auto start_pos = ImGui::GetCursorScreenPos();

		bool is_pressed        = ImGui::InvisibleButton("##Button", item_size, ImGuiButtonFlags_AllowOverlap);
		bool is_hovered        = ImGui::IsItemHovered();
		bool is_double_pressed = is_hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

		if (is_pressed && !is_double_pressed)
		{
			selecte_new_object(object);
		}
		else if (is_double_pressed)
		{
			if (Package* new_package = object->instance_cast<Package>())
			{
				m_selected_package = new_package;
			}
			else
			{
				if (auto client = ImGuiViewportClient::client_of(object->class_instance(), true))
				{
					client->select(object);
				}
			}

			on_object_double_click(object);
		}
		else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("ContentBrowser->Object", &object, sizeof(Object**));
			ImGui::Image(imgui_texture, image_size);
			ImGui::EndDragDropSource();
		}

		ImU32 color;

		if (object == selected_object)
		{
			auto active = ImGui::GetStyleColorVec4(ImGuiCol_FrameBgActive);

			if (is_hovered)
				active = ImGui::MakeHoveredColor(active);

			color = ImGui::GetColorU32(active);
		}
		else if (is_hovered)
		{
			color = ImGui::GetColorU32(ImGuiCol_FrameBgHovered);
		}
		else
		{
			color = ImGui::GetColorU32(ImGuiCol_FrameBg);
		}

		ImGui::SetCursorScreenPos(start_pos);
		ImGui::GetWindowDrawList()->AddRectFilled(start_pos, start_pos + item_size, color, ImGui::GetStyle().FrameRounding);
		{
			float border   = (item_size.x - image_size.x) / 2.f;
			auto image_min = start_pos + ImVec2(border, border);
			ImGui::GetWindowDrawList()->AddImageRounded(ImTextureID(imgui_texture), image_min, image_min + image_size, {0, 0},
			                                            {1, 1}, 0xFFFFFFFF, ImGui::GetStyle().FrameRounding);
		}

		ImGui::BeginVertical(object, item_size, 0.5);
		ImGui::Spring(0.f);
		ImGui::Dummy({item_size.x, item_size.x});

		ImGui::Spring(0.f);

		ImGui::BeginVertical(0, {image_size.x, 0}, 0.5);

		if (m_is_renaming && selected_object == object)
		{
			ImGui::SetNextItemWidth(image_size.x);
			bool modify = ImGui::InputText("##ObjectName", m_new_object_name, ImGuiInputTextFlags_EnterReturnsTrue);

			String validation;

			if (m_new_object_name == object->name().to_string())
			{
				// Nothing
			}
			else if (m_new_object_name.empty())
			{
				ImGui::PushStyleColor(ImGuiCol_Text, 0xFF0000FF);
				ImGui::SetTooltip("Please, provide a name for the asset!");
				ImGui::PopStyleColor();
			}
			else if (m_selected_package->contains_object(m_new_object_name))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, 0xFF0000FF);
				ImGui::SetTooltip("An asset already exist at this location with the name '%s'!", m_new_object_name.c_str());
				ImGui::PopStyleColor();
			}
			else if (!Object::static_validate_object_name(m_new_object_name, &validation))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, 0xFF0000FF);
				ImGui::SetTooltip("%s", validation.c_str());
				ImGui::PopStyleColor();
			}
			else if (modify)
			{
				bool status = selected_object->rename(m_new_object_name);
				if (status)
				{
					m_is_renaming = false;
				}
			}
		}
		else
		{
			ImGui::TextEllipsis(name.data(), image_size.x);
		}

		ImGui::EndVertical();

		ImGui::Spring(1.0);

		ImGui::BeginVertical(1, {image_size.x, 0}, 0.0);
		ImGui::PushFont(EditorTheme::small_font());
		ImGui::TextEllipsis(object->class_instance()->name().c_str(), image_size.x);
		ImGui::PopFont();
		ImGui::EndVertical();

		ImGui::Spring(0.f);
		ImGui::EndVertical();

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

		auto& objects = package->objects();

		const ImVec2 region     = ImGui::GetContentRegionAvail();
		const float font_size   = ImGui::GetFontSize();
		const ImVec2 item_size  = ImVec2(6.8, 6.8) * font_size + ImVec2(0.f, ImGui::GetTextLineHeightWithSpacing() * 3);
		const ImVec2 spacing    = ImGui::GetStyle().ItemSpacing;
		const int_t columns     = glm::max(static_cast<int_t>(region.x / (item_size.x + spacing.x)), 1);
		const int_t items_count = static_cast<int_t>(objects.size());
		const int_t rows        = (items_count + columns - 1) / columns;

		ImGuiListClipper clipper;
		clipper.Begin(rows, item_size.y + spacing.y + font_size);

		while (clipper.Step())
		{
			for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
			{
				ImGui::BeginHorizontal(row, ImVec2(region.x, item_size.y));
				int_t idx     = row * columns;
				int_t end_idx = glm::min(idx + columns, items_count);

				for (; idx < end_idx; ++idx)
				{
					auto object = objects[idx];
					ImGui::PushID(object);
					render_content_item(object, item_size);
					ImGui::PopID();
				}
				ImGui::EndHorizontal();
				ImGui::NewLine();
			}
		}

		clipper.End();

		if (ImGui::BeginPopup("###ContentContextMenu"))
		{
			Package* pkg = selected_package();

			if (ImGui::Button("editor/Create new asset"_localized))
			{
				ImGuiWindow::current()->widgets.create<ImGuiCreateNewAsset>(pkg, filters);
				ImGui::CloseCurrentPopup();
			}

			bool is_editable_object = selected_object && selected_object->is_editable();

			if (is_editable_object && ImGui::Button("editor/Rename"_localized))
			{
				begin_renaming(selected_object);
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

		if (ImGui::IsKeyPressed(ImGuiKey_F2, false))
		{
			if (!m_is_renaming && selected_object)
			{
				begin_renaming(selected_object);
			}
		}

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
		return "editor/Content Browser"_localized;
	}

	ContentBrowser::~ContentBrowser() {}
}// namespace Engine
