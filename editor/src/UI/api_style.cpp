#include "api_internal.hpp"
#include <UI/style.hpp>

namespace Trinex::UI
{
	static inline void push_style_counters(u32 colors, u32 vars)
	{
		auto& stack = active_context()->stack;
		stack.push<u32>(colors);
		stack.push<u32>(vars);
	}

	void push_style(const GlobalStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, value.alpha);
		ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, value.disabled_alpha);
		push_style_counters(0, 2);
	}

	void push_style(const TextStyle& value)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, to_imvec(value.color));
		ImGui::PushStyleColor(ImGuiCol_TextDisabled, to_imvec(value.disabled));
		ImGui::PushStyleColor(ImGuiCol_TextLink, to_imvec(value.link));
		ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, to_imvec(value.selection_bg));
		ImGui::PushStyleColor(ImGuiCol_InputTextCursor, to_imvec(value.cursor));
		push_style_counters(5, 0);
	}

	void push_style(const WindowStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, to_imvec(value.padding));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, to_imvec(value.min_size));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, to_imvec(value.title_align));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, value.rounding);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, value.border_size);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, to_imvec(with_alpha(value.bg, value.opacity)));
		ImGui::PushStyleColor(ImGuiCol_Border, to_imvec(value.border));
		ImGui::PushStyleColor(ImGuiCol_BorderShadow, to_imvec(value.border_shadow));
		ImGui::PushStyleColor(ImGuiCol_TitleBg, to_imvec(value.title_bg));
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, to_imvec(value.title_bg_active));
		ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, to_imvec(value.title_bg_collapsed));
		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, to_imvec(value.menu_bar_bg));
		ImGui::PushStyleColor(ImGuiCol_ResizeGrip, to_imvec(value.resize_grip));
		ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, to_imvec(value.resize_grip_hovered));
		ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, to_imvec(value.resize_grip_active));
		push_style_counters(10, 5);
	}

	void push_style(const ChildStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, value.rounding);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, value.border_size);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, to_imvec(with_alpha(value.bg, value.opacity)));
		push_style_counters(1, 2);
	}

	void push_style(const PopupStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, value.rounding);
		ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, value.border_size);
		ImGui::PushStyleColor(ImGuiCol_PopupBg, to_imvec(with_alpha(value.bg, value.opacity)));
		push_style_counters(1, 2);
	}

	void push_style(const LayoutStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, to_imvec(value.item_spacing));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, to_imvec(value.item_inner_spacing));
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, value.indent_spacing);
		push_style_counters(0, 3);
	}

	void push_style(const FrameStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, to_imvec(value.padding));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, value.rounding);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, value.border_size);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, to_imvec(value.bg));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, to_imvec(value.bg_hovered));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, to_imvec(value.bg_active));
		push_style_counters(3, 3);
	}

	void push_style(const ButtonStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, to_imvec(value.text_align));
		ImGui::PushStyleColor(ImGuiCol_Button, to_imvec(value.bg));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, to_imvec(value.bg_hovered));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, to_imvec(value.bg_active));
		push_style_counters(3, 1);
	}

	void push_style(const MarkStyle& value)
	{
		ImGui::PushStyleColor(ImGuiCol_CheckMark, to_imvec(value.check_mark));
		push_style_counters(1, 0);
	}

	void push_style(const HeaderStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, to_imvec(value.selectable_text_align));
		ImGui::PushStyleColor(ImGuiCol_Header, to_imvec(value.bg));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, to_imvec(value.bg_hovered));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, to_imvec(value.bg_active));
		push_style_counters(3, 1);
	}

	void push_style(const ScrollbarStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, value.size);
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, value.rounding);
		ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, to_imvec(value.bg));
		ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, to_imvec(value.grab));
		ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, to_imvec(value.grab_hovered));
		ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, to_imvec(value.grab_active));
		push_style_counters(4, 2);
	}

	void push_style(const GrabStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, value.min_size);
		ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, value.rounding);
		ImGui::PushStyleColor(ImGuiCol_SliderGrab, to_imvec(value.slider_grab));
		ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, to_imvec(value.slider_grab_active));
		push_style_counters(2, 2);
	}

	void push_style(const SeparatorStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, value.text_border_size);
		ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextAlign, to_imvec(value.text_align));
		ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, to_imvec(value.text_padding));
		ImGui::PushStyleColor(ImGuiCol_Separator, to_imvec(value.color));
		ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, to_imvec(value.hovered));
		ImGui::PushStyleColor(ImGuiCol_SeparatorActive, to_imvec(value.active));
		push_style_counters(3, 3);
	}

	void push_style(const TabStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, value.rounding);
		ImGui::PushStyleVar(ImGuiStyleVar_TabBorderSize, value.border_size);
		ImGui::PushStyleVar(ImGuiStyleVar_TabMinWidthBase, value.min_width_base);
		ImGui::PushStyleVar(ImGuiStyleVar_TabMinWidthShrink, value.min_width_shrink);
		ImGui::PushStyleVar(ImGuiStyleVar_TabBarBorderSize, value.bar_border_size);
		ImGui::PushStyleVar(ImGuiStyleVar_TabBarOverlineSize, value.bar_overline_size);
		ImGui::PushStyleColor(ImGuiCol_Tab, to_imvec(value.bg));
		ImGui::PushStyleColor(ImGuiCol_TabHovered, to_imvec(value.bg_hovered));
		ImGui::PushStyleColor(ImGuiCol_TabSelected, to_imvec(value.bg_selected));
		ImGui::PushStyleColor(ImGuiCol_TabSelectedOverline, to_imvec(value.selected_overline));
		ImGui::PushStyleColor(ImGuiCol_TabDimmed, to_imvec(value.bg_dimmed));
		ImGui::PushStyleColor(ImGuiCol_TabDimmedSelected, to_imvec(value.bg_dimmed_selected));
		ImGui::PushStyleColor(ImGuiCol_TabDimmedSelectedOverline, to_imvec(value.dimmed_selected_overline));
		push_style_counters(7, 6);
	}

	void push_style(const TableStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, to_imvec(value.cell_padding));
		ImGui::PushStyleVar(ImGuiStyleVar_TableAngledHeadersAngle, value.angled_headers_angle);
		ImGui::PushStyleVar(ImGuiStyleVar_TableAngledHeadersTextAlign, to_imvec(value.angled_headers_text_align));
		ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, to_imvec(value.header_bg));
		ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, to_imvec(value.border_strong));
		ImGui::PushStyleColor(ImGuiCol_TableBorderLight, to_imvec(value.border_light));
		ImGui::PushStyleColor(ImGuiCol_TableRowBg, to_imvec(value.row_bg));
		ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, to_imvec(value.row_bg_alt));
		push_style_counters(5, 3);
	}

	void push_style(const PlotStyle& value)
	{
		ImGui::PushStyleColor(ImGuiCol_PlotLines, to_imvec(value.lines));
		ImGui::PushStyleColor(ImGuiCol_PlotLinesHovered, to_imvec(value.lines_hovered));
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, to_imvec(value.histogram));
		ImGui::PushStyleColor(ImGuiCol_PlotHistogramHovered, to_imvec(value.histogram_hovered));
		push_style_counters(4, 0);
	}

	void push_style(const TreeStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_TreeLinesSize, value.lines_size);
		ImGui::PushStyleVar(ImGuiStyleVar_TreeLinesRounding, value.lines_rounding);
		ImGui::PushStyleColor(ImGuiCol_TreeLines, to_imvec(value.lines));
		push_style_counters(1, 2);
	}

	void push_style(const DockingStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_DockingSeparatorSize, value.separator_size);
		ImGui::PushStyleColor(ImGuiCol_DockingPreview, to_imvec(value.preview));
		ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, to_imvec(value.empty_bg));
		push_style_counters(2, 1);
	}

	void push_style(const NavigationStyle& value)
	{
		ImGui::PushStyleColor(ImGuiCol_NavCursor, to_imvec(value.cursor));
		ImGui::PushStyleColor(ImGuiCol_NavWindowingHighlight, to_imvec(value.windowing_highlight));
		ImGui::PushStyleColor(ImGuiCol_NavWindowingDimBg, to_imvec(value.windowing_dim_bg));
		ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, to_imvec(value.modal_dim_bg));
		push_style_counters(4, 0);
	}

	void push_style(const DragDropStyle& value)
	{
		ImGui::PushStyleColor(ImGuiCol_DragDropTarget, to_imvec(value.target));
		push_style_counters(1, 0);
	}

	void push_style(const ImageStyle& value)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ImageBorderSize, value.border_size);
		push_style_counters(0, 1);
	}

	void pop_style()
	{
		auto& stack = active_context()->stack;
		ImGui::PopStyleVar(*stack.pop<u32>());
		ImGui::PopStyleColor(*stack.pop<u32>());
	}
}// namespace Trinex::UI
