#include <UI/Elements/navigation.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	static Element::UpdateFlags handle_click(Element* element, bool clicked, const Name& event)
	{
		if (clicked)
		{
			element->dispatch(event);
		}

		return Element::item_state_flags(Element::readback_if(clicked));
	}

	static ImVec2 to_imgui_size(Size size)
	{
		return ImVec2(size.width.value, size.height.value);
	}

	trinex_implement_ui_element(CollapsingHeader)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("right_text", &This::right_text);
		reflection()->bind("default_open", &This::default_open);
		reflection()->bind("disabled", &This::disabled);
		trinex_ui_bind_property(header_color, Style);
	}

	CollapsingHeader& CollapsingHeader::push_style()
	{
		Super::push_style();
		push_style_color(ImGuiCol_Header, header_color);
		push_style_color(ImGuiCol_HeaderHovered, header_color);
		push_style_color(ImGuiCol_HeaderActive, header_color);
		return *this;
	}

	CollapsingHeader& CollapsingHeader::pop_style()
	{
		ImGui::PopStyleColor(3);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags CollapsingHeader::on_begin_update()
	{
		ImGuiTreeNodeFlags flags = default_open ? ImGuiTreeNodeFlags_DefaultOpen : 0;
		if (ImGui::CollapsingHeader(label.c_str(), flags))
		{
			return UpdateFlags::Default;
		}

		return UpdateFlags::Undefined;
	}

	Element& CollapsingHeader::on_end_update(UpdateFlags flags)
	{
		return *this;
	}

	trinex_implement_ui_element(SectionHeader)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("right_text", &This::right_text);
		reflection()->bind("default_open", &This::default_open);
		reflection()->bind("disabled", &This::disabled);
		trinex_ui_bind_property(header_color, Style);
	}

	SectionHeader& SectionHeader::push_style()
	{
		Super::push_style();
		push_style_color(ImGuiCol_Header, header_color);
		push_style_color(ImGuiCol_HeaderHovered, header_color);
		push_style_color(ImGuiCol_HeaderActive, header_color);
		return *this;
	}

	SectionHeader& SectionHeader::pop_style()
	{
		ImGui::PopStyleColor(3);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags SectionHeader::on_begin_update()
	{
		ImGuiTreeNodeFlags flags = default_open ? ImGuiTreeNodeFlags_DefaultOpen : 0;
		if (ImGui::CollapsingHeader(label.c_str(), flags))
		{
			return UpdateFlags::Default;
		}

		return UpdateFlags::Undefined;
	}

	Element& SectionHeader::on_end_update(UpdateFlags flags)
	{
		return *this;
	}

	trinex_implement_ui_element(TreeNode)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("badge", &This::badge);
		reflection()->bind("default_open", &This::default_open);
		reflection()->bind("selected", &This::selected);
		trinex_ui_bind_property(line_size, Style);
		trinex_ui_bind_property(line_rounding, Style);
		trinex_ui_bind_property(header_color, Style);
		trinex_ui_bind_property(line_color, Style);
	}

	TreeNode& TreeNode::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_TreeLinesSize, line_size);
		push_style_var(ImGuiStyleVar_TreeLinesRounding, line_rounding);
		push_style_color(ImGuiCol_Header, header_color);
		push_style_color(ImGuiCol_HeaderHovered, header_color);
		push_style_color(ImGuiCol_HeaderActive, header_color);
		push_style_color(ImGuiCol_TreeLines, line_color);
		return *this;
	}

	TreeNode& TreeNode::pop_style()
	{
		ImGui::PopStyleColor(4);
		ImGui::PopStyleVar(2);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags TreeNode::on_begin_update()
	{
		ImGuiTreeNodeFlags flags = default_open ? ImGuiTreeNodeFlags_DefaultOpen : 0;
		if (selected)
		{
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		if (ImGui::TreeNodeEx(label.c_str(), flags))
		{
			return UpdateFlags::Default;
		}

		return UpdateFlags::End;
	}

	Element& TreeNode::on_end_update(UpdateFlags flags)
	{
		ImGui::TreePop();
		return *this;
	}

	trinex_implement_ui_element(TreeLeaf)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("badge", &This::badge);
		reflection()->bind("selected", &This::selected);
		reflection()->bind("on_click", &This::on_click);
		trinex_ui_bind_property(rounding, Style);
		trinex_ui_bind_property(text_align, Style);
		trinex_ui_bind_property(background_color, Style);
	}

	TreeLeaf& TreeLeaf::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_SelectableRounding, rounding);
		push_style_var(ImGuiStyleVar_SelectableTextAlign, text_align);
		push_style_color(ImGuiCol_Header, background_color);
		push_style_color(ImGuiCol_HeaderHovered, background_color);
		push_style_color(ImGuiCol_HeaderActive, background_color);
		return *this;
	}

	TreeLeaf& TreeLeaf::pop_style()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(2);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags TreeLeaf::on_begin_update()
	{
		return handle_click(this, ImGui::Selectable(label.c_str(), selected), on_click);
	}

	trinex_implement_ui_element(TabBar)
	{
		trinex_ui_bind_property(border_size, Style);
		trinex_ui_bind_property(overline_size, Style);
	}

	TabBar& TabBar::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_TabBarBorderSize, border_size);
		push_style_var(ImGuiStyleVar_TabBarOverlineSize, overline_size);
		return *this;
	}

	TabBar& TabBar::pop_style()
	{
		ImGui::PopStyleVar(2);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags TabBar::on_begin_update()
	{
		ImGui::PushID(this);
		if (ImGui::BeginTabBar(id().is_valid() ? id().c_str() : "##TabBar"))
		{
			return UpdateFlags::Default;
		}

		ImGui::PopID();
		return UpdateFlags::Undefined;
	}

	Element& TabBar::on_end_update(UpdateFlags flags)
	{
		ImGui::EndTabBar();
		ImGui::PopID();
		return *this;
	}

	trinex_implement_ui_element(Tab)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("selected", &This::selected);
		reflection()->bind("size", &This::size);
		reflection()->bind("on_click", &This::on_click);
		trinex_ui_bind_property(rounding, Style);
		trinex_ui_bind_property(border_size, Style);
		trinex_ui_bind_property(min_width_base, Style);
		trinex_ui_bind_property(min_width_shrink, Style);
		trinex_ui_bind_property(background_color, Style);
		trinex_ui_bind_property(overline_color, Style);
	}

	Tab& Tab::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_TabRounding, rounding);
		push_style_var(ImGuiStyleVar_TabBorderSize, border_size);
		push_style_var(ImGuiStyleVar_TabMinWidthBase, min_width_base);
		push_style_var(ImGuiStyleVar_TabMinWidthShrink, min_width_shrink);
		push_style_color(ImGuiCol_Tab, background_color);
		push_style_color(ImGuiCol_TabHovered, background_color);
		push_style_color(ImGuiCol_TabSelected, background_color);
		push_style_color(ImGuiCol_TabDimmed, background_color);
		push_style_color(ImGuiCol_TabDimmedSelected, background_color);
		push_style_color(ImGuiCol_TabSelectedOverline, overline_color);
		push_style_color(ImGuiCol_TabDimmedSelectedOverline, overline_color);
		return *this;
	}

	Tab& Tab::pop_style()
	{
		ImGui::PopStyleColor(7);
		ImGui::PopStyleVar(4);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags Tab::on_begin_update()
	{
		bool open          = true;
		const bool visible = ImGui::BeginTabItem(label.c_str(), &open);
		const bool clicked = ImGui::IsItemClicked();
		if (visible)
		{
			ImGui::EndTabItem();
		}
		return handle_click(this, clicked, on_click);
	}

	trinex_implement_ui_element(SidebarItem)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("badge", &This::badge);
		reflection()->bind("selected", &This::selected);
		reflection()->bind("on_click", &This::on_click);
		trinex_ui_bind_property(rounding, Style);
		trinex_ui_bind_property(text_align, Style);
		trinex_ui_bind_property(background_color, Style);
	}

	SidebarItem& SidebarItem::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_SelectableRounding, rounding);
		push_style_var(ImGuiStyleVar_SelectableTextAlign, text_align);
		push_style_color(ImGuiCol_Header, background_color);
		push_style_color(ImGuiCol_HeaderHovered, background_color);
		push_style_color(ImGuiCol_HeaderActive, background_color);
		return *this;
	}

	SidebarItem& SidebarItem::pop_style()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(2);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags SidebarItem::on_begin_update()
	{
		return handle_click(this, ImGui::Selectable(label.c_str(), selected), on_click);
	}

	trinex_implement_ui_element(NavItem)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("icon", &This::icon);
		reflection()->bind("selected", &This::selected);
		reflection()->bind("on_click", &This::on_click);
		trinex_ui_bind_property(rounding, Style);
		trinex_ui_bind_property(text_align, Style);
		trinex_ui_bind_property(background_color, Style);
	}

	NavItem& NavItem::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_SelectableRounding, rounding);
		push_style_var(ImGuiStyleVar_SelectableTextAlign, text_align);
		push_style_color(ImGuiCol_Header, background_color);
		push_style_color(ImGuiCol_HeaderHovered, background_color);
		push_style_color(ImGuiCol_HeaderActive, background_color);
		return *this;
	}

	NavItem& NavItem::pop_style()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(2);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags NavItem::on_begin_update()
	{
		return handle_click(this, ImGui::Selectable(label.c_str(), selected), on_click);
	}

	trinex_implement_ui_element(Breadcrumb)
	{
		reflection()->bind("label", &This::label);
		reflection()->bind("current", &This::current);
		reflection()->bind("on_click", &This::on_click);
		trinex_ui_bind_property(rounding, Style);
		trinex_ui_bind_property(text_align, Style);
		trinex_ui_bind_property(background_color, Style);
	}

	Breadcrumb& Breadcrumb::push_style()
	{
		Super::push_style();
		push_style_var(ImGuiStyleVar_SelectableRounding, rounding);
		push_style_var(ImGuiStyleVar_SelectableTextAlign, text_align);
		push_style_color(ImGuiCol_Header, background_color);
		push_style_color(ImGuiCol_HeaderHovered, background_color);
		push_style_color(ImGuiCol_HeaderActive, background_color);
		return *this;
	}

	Breadcrumb& Breadcrumb::pop_style()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(2);
		return *Super::pop_style().as<This>();
	}

	Element::UpdateFlags Breadcrumb::on_begin_update()
	{
		if (!current)
		{
			ImGui::TextUnformatted("/");
			ImGui::SameLine();
		}
		return handle_click(this, ImGui::Selectable(label.c_str(), current, 0, to_imgui_size(Size(0.0f, 0.0f))), on_click);
	}
}// namespace Trinex::UI
