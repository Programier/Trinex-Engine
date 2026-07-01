#include <Core/constants.hpp>
#include <Core/filesystem/directory_iterator.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/icons.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <Editor/Widgets/content_browser.hpp>
#include <Engine/project.hpp>
#include <Graphics/texture.hpp>
#include <RHI/static_sampler.hpp>
#include <UI/api.hpp>
#include <Widgets/imgui_windows.hpp>

#include <cstdio>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace Trinex
{
	namespace
	{
		static void text_ellipsis(const char* text, float max_width)
		{
			ImGui::TextEllipsis(text, max_width);
		}

		static void sync_string_buffer(char* buffer, usize size, String& value)
		{
			if (buffer == nullptr || size == 0)
			{
				return;
			}

			std::snprintf(buffer, size, "%s", value.c_str());
		}
	}// namespace

	ContentBrowserWidget::ContentBrowserWidget()
	    : UI::Widget(ContentBrowserWidget::static_name()), m_selected_package(Object::root_package())
	{}

	void ContentBrowserWidget::selecte_new_object(Object* object)
	{
		selected_object = object;
		on_object_select(object);
		m_is_renaming = false;
	}

	void ContentBrowserWidget::begin_renaming(Object* object)
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

	bool ContentBrowserWidget::render_package_popup(void* data)
	{
		bool is_editable = (m_show_popup_for && m_show_popup_for->is_editable() && m_show_popup_for->is_serializable());

		if (UI::button("editor/Create Package"_localized))
		{
			//ImGuiWindow::current()->widgets.create_identified<ImGuiCreateNewPackage>(this, m_show_popup_for);
			return false;
		}

		if (is_editable && UI::button("editor/Rename"_localized))
		{
			//ImGuiWindow::current()->widgets.create_identified<ImGuiRenameObject>("RenameObject", m_selected_package);
			return false;
		}

		if (is_editable && UI::button("editor/Save"_localized))
		{
			m_show_popup_for->save();
			return false;
		}

		return true;
	}

	void ContentBrowserWidget::render_package_popup()
	{
		UI::popup("###PackagePopup", [&]() {
			bool is_editable = (m_show_popup_for && m_show_popup_for->is_editable() && m_show_popup_for->is_serializable());

			if (UI::button("editor/Create Package"_localized))
			{
				ImGuiWindow::current()->widgets.create_identified<ImGuiCreateNewPackage>(this, m_show_popup_for);
				UI::close_popup();
			}

			if (is_editable && UI::button("editor/Rename"_localized))
			{
				ImGuiWindow::current()->widgets.create_identified<ImGuiRenameObject>("RenameObject", m_selected_package);
				UI::close_popup();
			}

			if (is_editable && UI::button("editor/Save"_localized))
			{
				m_show_popup_for->save();
				UI::close_popup();
			}
		});
	}

	void ContentBrowserWidget::render_package_tree(Package* node)
	{
		if (node == nullptr)
		{
			return;
		}

		const bool is_root_package = node == Object::root_package();
		UI::TreeNodeOptions options;
		options.default_open = is_root_package;
		options.selected     = node == m_selected_package;

		UI::push_id(node);
		const bool opened = UI::tree_node(node->string_name().c_str(), options);
		if (UI::is_item_clicked())
		{
			m_selected_package = node;
		}

		if (UI::begin_context_menu())
		{
			m_selected_package = node;
			m_show_popup_for   = node;

			const bool is_editable = (m_show_popup_for && m_show_popup_for->is_editable() && m_show_popup_for->is_serializable());
			if (UI::button("editor/Create Package"_localized))
			{
				ImGuiWindow::current()->widgets.create_identified<ImGuiCreateNewPackage>(this, m_show_popup_for);
				UI::close_popup();
			}

			if (is_editable && UI::button("editor/Rename"_localized))
			{
				ImGuiWindow::current()->widgets.create_identified<ImGuiRenameObject>("RenameObject", m_selected_package);
				UI::close_popup();
			}

			if (is_editable && UI::button("editor/Save"_localized))
			{
				m_show_popup_for->save();
				UI::close_popup();
			}

			UI::end_context_menu();
		}

		if (opened)
		{
			for (auto child : node->objects())
			{
				if (Package* pkg = child->instance_cast<Package>())
				{
					render_package_tree(pkg);
				}
			}
			UI::tree_pop();
		}

		UI::pop_id();
	}

	void ContentBrowserWidget::render_packages()
	{
		if (!UI::begin_panel("##PGS", {}))
		{
			return;
		}

		if (UI::icon_button(ICON_LC_PLUS, "###add"))
		{
			ImGuiOpenFile::Flags flags = ImGuiOpenFile::MultipleSelection;
			auto window                = ImGuiWindow::current()->widgets.create_identified<ImGuiOpenFile>(this, flags);
			window->on_select.push([](const Path& path) {
				Path relative = path.relative(rootfs()->native_path(Project::assets_dir));
				Object::load_object_from_file(relative);
			});
			window->type_filters({Constants::asset_extention});
			window->pwd(Project::assets_dir);
		}

		UI::same_line();
		render_package_tree(Object::root_package());

		UI::end_panel();
	}

	bool ContentBrowserWidget::render_content_item(Object* object, const UI::Vec2& size)
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

		return UI::card_button(object->name().c_str(), {.size = size}, [&]() {
			const float padding       = ImGui::GetStyle().WindowPadding.x;
			UI::Vec2 content_size     = {glm::max(size.x - padding * 2.0f, 0.0f), glm::max(size.y - padding * 2.0f, 0.0f)};
			ImTextureID imgui_texture = Icons::find_icon(object);
			imgui_texture.sampler     = RHIPointWrapSampler::static_sampler();

			const float image_side_length = content_size.x * 0.93f;
			const UI::Vec2 image_size     = {image_side_length, image_side_length};

			const auto start_pos = UI::cursor_screen_position();
			bool is_pressed = UI::invisible_button("##Button", {.size = content_size, .flags = UI::ButtonFlags::AllowOverlap});
			bool is_hovered = UI::is_item_hovered();
			bool is_double_pressed = is_hovered && UI::is_mouse_double_clicked(UI::MouseButton::Left);

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
					// if (auto client = ImGuiViewportClient::client_of(object->class_instance(), true))
					// {
					// 	client->select(object);
					// }
				}

				on_object_double_click(object);
			}
			else if (UI::is_mouse_dragging() && UI::begin_drag_source())
			{
				UI::drag_payload("ContentBrowserWidget->Object", &object, sizeof(Object**));
				UI::image(imgui_texture.texture, image_size);
				UI::end_drag_source();
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

			UI::cursor_screen_position(start_pos);
			UI::draw_list()->fill_rect(start_pos, start_pos + content_size, ImGui::GetStyle().FrameRounding);

			{
				float border   = (content_size.x - image_size.x) / 2.f;
				auto image_min = start_pos + UI::Vec2(border, border);
				UI::draw_list()->rounded_image(UI::Texture(imgui_texture.texture, imgui_texture.sampler), image_min,
				                               image_min + image_size, {0, 0}, {1, 1}, 0xFFFFFFFF,
				                               ImGui::GetStyle().FrameRounding);
			}

			UI::begin_vertical(object, UI::Vec2{content_size.x, content_size.y}, 0.5f);
			UI::spring(0.f);
			UI::dummy({content_size.x, content_size.x});

			UI::spring(0.f);
			UI::begin_vertical(0, {image_size.x, 0}, 0.5f);

			if (m_is_renaming && selected_object == object)
			{
				char rename_buffer[512];
				sync_string_buffer(rename_buffer, sizeof(rename_buffer), m_new_object_name);
				bool modify       = UI::input("##ObjectName", rename_buffer, sizeof(rename_buffer), UI::Vec2(image_size.x, 0.0f),
				                              UI::InputTextFlags::EnterReturnsTrue);
				m_new_object_name = rename_buffer;

				String validation;

				if (m_new_object_name == object->name().to_string())
				{
					// Nothing
				}
				else if (m_new_object_name.empty())
				{
					UI::tooltip("Please, provide a name for the asset!");
				}
				else if (m_selected_package->contains_object(m_new_object_name))
				{
					UI::tooltip(Strings::format("An asset already exist at this location with the name '{}'!", m_new_object_name)
					                    .c_str());
				}
				else if (!Object::static_validate_object_name(m_new_object_name, &validation))
				{
					UI::tooltip(validation.c_str());
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
				text_ellipsis(name.data(), image_size.x);
			}

			UI::end_vertical();

			UI::spring(1.0f);

			UI::begin_vertical(1, {image_size.x, 0}, 0.0f);
			UI::push_text_font(UI::FontSize::Small);
			text_ellipsis(object->class_instance()->name().c_str(), image_size.x);
			UI::pop_font();
			UI::end_vertical();

			UI::spring(0.f);
			UI::end_vertical();
		});
	}

	static Package* render_package_path(Package* package)
	{
		if (package == nullptr)
			return nullptr;

		Vector<Package*> chain;
		for (Package* current = package; current; current = current->package())
		{
			chain.push_back(current);
		}

		Package* result = nullptr;
		for (auto it = chain.rbegin(); it != chain.rend(); ++it)
		{
			const bool current = it + 1 == chain.rend();
			if (UI::breadcrumb((*it)->name().c_str(), current))
			{
				result = *it;
			}
		}
		return result;
	}

	void ContentBrowserWidget::render_content_window()
	{
		UI::WindowOptions options;
		options.flags = UI::WindowFlags::MenuBar;

		if (!UI::begin_panel("##ITEMS", {}))
		{
			return;
		}

		// UI::menu_bar([&]() {
		// 	if (Package* new_package = render_package_path(m_selected_package))
		// 	{
		// 		m_selected_package = new_package;
		// 	}
		// });

		Package* package = m_selected_package;

		if (package == nullptr)
		{
			UI::text("editor/No package selected!"_localized);
			UI::end_window();
			return;
		}

		if (UI::is_window_hovered() && UI::is_mouse_clicked(UI::MouseButton::Right))
		{
			UI::open_popup("###ContentContextMenu");
		}

		auto& objects = package->objects();

		Vector<Object*> visible_objects;
		visible_objects.reserve(objects.size());
		for (Object* object : objects)
		{
			bool in_filter = filters.empty();
			if (!in_filter)
			{
				for (auto& callback : filters)
				{
					in_filter = callback(object->class_instance());
					if (in_filter)
					{
						break;
					}
				}
			}

			if (in_filter)
			{
				visible_objects.push_back(object);
			}
		}

		const auto region       = UI::content_region_available();
		const float font_size   = ImGui::GetFontSize();
		const ImVec2 item_size  = ImVec2(8.8, 10.8) * font_size + ImVec2(0.f, ImGui::GetTextLineHeightWithSpacing() * 3);
		const ImVec2 spacing    = ImGui::GetStyle().ItemSpacing;
		const float column_step = item_size.x + spacing.x;
		const float row_height  = item_size.y + spacing.y;
		const i32 columns       = glm::max(static_cast<i32>((region.x + spacing.x) / column_step), 1);
		const i32 items_count   = static_cast<i32>(visible_objects.size());
		const i32 rows          = (items_count + columns - 1) / columns;

		UI::TableFlags table_flags = UI::TableFlags::SizingFixedSame | UI::TableFlags::NoSavedSettings |
		                             UI::TableFlags::NoPadOuterX | UI::TableFlags::NoPadInnerX;


		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.f, 0.f));

		if (UI::begin_table("##content_grid", columns, table_flags, region))
		{
			for (i32 column = 0; column < columns; ++column)
			{
				UI::table_setup_column("", UI::TableColumnFlags::WidthFixed, column_step);
			}

			ImGuiListClipper clipper;
			clipper.Begin(rows, row_height);

			while (clipper.Step())
			{
				for (i32 row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
				{
					UI::table_next_row(UI::TableRowFlags::Undefined, row_height);

					for (i32 column = 0; column < columns; ++column)
					{
						const i32 index = row * columns + column;
						if (index >= items_count)
						{
							break;
						}

						UI::table_column(column);
						Object* object = visible_objects[index];
						UI::push_id(object);
						render_content_item(object, {item_size.x, item_size.y});
						UI::pop_id();
					}
				}
			}

			clipper.End();
			UI::end_table();
		}

		ImGui::PopStyleVar();

		UI::popup("###ContentContextMenu", [&]() {
			Package* pkg = selected_package();

			if (UI::button("editor/Create new asset"_localized))
			{
				ImGuiWindow::current()->widgets.create<ImGuiCreateNewAsset>(pkg, filters);
				UI::close_popup();
			}

			bool is_editable_object = selected_object && selected_object->is_editable();

			if (is_editable_object && UI::button("editor/Rename"_localized))
			{
				begin_renaming(selected_object);
				UI::close_popup();
			}

			if (is_editable_object && UI::button("editor/Delete"_localized))
			{
				Package* package = selected_object->package();
				package->remove_object(selected_object);
				selected_object = nullptr;
				on_object_select(nullptr);
				UI::close_popup();
			}

			if (is_editable_object && UI::button("editor/Save"_localized))
			{
				selected_object->save();
				UI::close_popup();
			}
		});

		UI::end_panel();
	}

	void ContentBrowserWidget::on_render()
	{
		if (UI::is_key_pressed(UI::Key::F2, false))
		{
			if (!m_is_renaming && selected_object)
			{
				begin_renaming(selected_object);
			}
		}

		UI::TableFlags flags = UI::TableFlags::Resizable | UI::TableFlags::BordersInnerV | UI::TableFlags::SizingStretchProp;

		if (UI::begin_table("##dock", 2, flags, UI::content_region_available()))
		{
			UI::table_setup_column("###packages", UI::TableFlags::Undefined, 0.3f);
			UI::table_setup_column("###content", UI::TableFlags::Undefined, 0.7f);

			UI::table_next_row();
			UI::table_column(0);
			render_packages();
			UI::table_column(1);
			render_content_window();
			UI::end_table();
		}
	}

	Package* ContentBrowserWidget::selected_package() const
	{
		return m_selected_package;
	}

	const char* ContentBrowserWidget::static_name()
	{
		return "editor/Content Browser"_localized;
	}

	ContentBrowserWidget::~ContentBrowserWidget() {}
}// namespace Trinex
