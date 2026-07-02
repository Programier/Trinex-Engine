#pragma once
#include <UI/core.hpp>

namespace Trinex::UI
{
	// ============================================================================
	// Global
	// ============================================================================

	struct GlobalStyle {
		// ImGuiStyleVar_Alpha
		f32 alpha = 1.0f;

		// ImGuiStyleVar_DisabledAlpha
		f32 disabled_alpha = 0.6f;
	};

	// ============================================================================
	// Text
	// ============================================================================

	struct TextStyle {
		// ImGuiCol_Text
		Color color{};

		// ImGuiCol_TextDisabled
		Color disabled{};

		// ImGuiCol_TextLink
		Color link{};

		// ImGuiCol_TextSelectedBg
		Color selection_bg{};

		// ImGuiCol_InputTextCursor
		Color cursor{};
	};

	// ============================================================================
	// Window
	// ============================================================================

	struct WindowStyle {
		// ImGuiStyleVar_WindowPadding
		Vec2 padding{};

		// ImGuiStyleVar_WindowMinSize
		Vec2 min_size{};

		// ImGuiStyleVar_WindowTitleAlign
		Vec2 title_align{};

		// ImGuiStyleVar_WindowRounding
		f32 rounding = 0.0f;

		// ImGuiStyleVar_WindowBorderSize
		f32 border_size = 0.0f;

		// Logical opacity for WindowBg alpha.
		// Not direct ImGuiStyleVar. You can apply it to bg.w.
		f32 opacity = 1.0f;

		// ImGuiCol_WindowBg
		Color bg{};

		// ImGuiCol_Border
		Color border{};

		// ImGuiCol_BorderShadow
		Color border_shadow{};

		// ImGuiCol_TitleBg
		Color title_bg{};

		// ImGuiCol_TitleBgActive
		Color title_bg_active{};

		// ImGuiCol_TitleBgCollapsed
		Color title_bg_collapsed{};

		// ImGuiCol_MenuBarBg
		Color menu_bar_bg{};

		// ImGuiCol_ResizeGrip
		Color resize_grip{};

		// ImGuiCol_ResizeGripHovered
		Color resize_grip_hovered{};

		// ImGuiCol_ResizeGripActive
		Color resize_grip_active{};
	};

	// ============================================================================
	// Child Window
	// ============================================================================

	struct ChildStyle {
		// ImGuiStyleVar_ChildRounding
		f32 rounding = 0.0f;

		// ImGuiStyleVar_ChildBorderSize
		f32 border_size = 0.0f;

		// Logical opacity for ChildBg alpha.
		f32 opacity = 1.0f;

		// ImGuiCol_ChildBg
		Color bg{};
	};

	// ============================================================================
	// Popup / Tooltip / Menu Popup
	// ============================================================================

	struct PopupStyle {
		// ImGuiStyleVar_PopupRounding
		f32 rounding = 0.0f;

		// ImGuiStyleVar_PopupBorderSize
		f32 border_size = 0.0f;

		// Logical opacity for PopupBg alpha.
		f32 opacity = 1.0f;

		// ImGuiCol_PopupBg
		Color bg{};
	};

	// ============================================================================
	// Layout
	// ============================================================================

	struct LayoutStyle {
		// ImGuiStyleVar_ItemSpacing
		Vec2 item_spacing{};

		// ImGuiStyleVar_ItemInnerSpacing
		Vec2 item_inner_spacing{};

		// ImGuiStyleVar_IndentSpacing
		f32 indent_spacing = 0.0f;
	};

	// ============================================================================
	// Frame
	// For InputText, Combo, Slider frame bg, Checkbox frame, RadioButton frame etc.
	// ============================================================================

	struct FrameStyle {
		// ImGuiStyleVar_FramePadding
		Vec2 padding{};

		// ImGuiStyleVar_FrameRounding
		f32 rounding = 0.0f;

		// ImGuiStyleVar_FrameBorderSize
		f32 border_size = 0.0f;

		// ImGuiCol_FrameBg
		Color bg{};

		// ImGuiCol_FrameBgHovered
		Color bg_hovered{};

		// ImGuiCol_FrameBgActive
		Color bg_active{};
	};

	// ============================================================================
	// Button
	// ============================================================================

	struct ButtonStyle {
		// ImGuiStyleVar_ButtonTextAlign
		Vec2 text_align{0.5f, 0.5f};

		// ImGuiCol_Button
		Color bg{};

		// ImGuiCol_ButtonHovered
		Color bg_hovered{};

		// ImGuiCol_ButtonActive
		Color bg_active{};
	};

	// ============================================================================
	// Checkbox / Radio / Markers
	// ============================================================================

	struct MarkStyle {
		// ImGuiCol_CheckMark
		Color check_mark{};
	};

	// ============================================================================
	// Header / Selectable / TreeNode / MenuItem
	// ============================================================================

	struct HeaderStyle {
		// ImGuiStyleVar_SelectableTextAlign
		Vec2 selectable_text_align{0.0f, 0.0f};

		// ImGuiCol_Header
		Color bg{};

		// ImGuiCol_HeaderHovered
		Color bg_hovered{};

		// ImGuiCol_HeaderActive
		Color bg_active{};
	};

	// ============================================================================
	// Scrollbar
	// ============================================================================

	struct ScrollbarStyle {
		// ImGuiStyleVar_ScrollbarSize
		f32 size = 0.0f;

		// ImGuiStyleVar_ScrollbarRounding
		f32 rounding = 0.0f;

		// ImGuiCol_ScrollbarBg
		Color bg{};

		// ImGuiCol_ScrollbarGrab
		Color grab{};

		// ImGuiCol_ScrollbarGrabHovered
		Color grab_hovered{};

		// ImGuiCol_ScrollbarGrabActive
		Color grab_active{};
	};

	// ============================================================================
	// Slider / Drag Grab
	// ============================================================================

	struct GrabStyle {
		// ImGuiStyleVar_GrabMinSize
		f32 min_size = 0.0f;

		// ImGuiStyleVar_GrabRounding
		f32 rounding = 0.0f;

		// ImGuiCol_SliderGrab
		Color slider_grab{};

		// ImGuiCol_SliderGrabActive
		Color slider_grab_active{};
	};

	// ============================================================================
	// Separator
	// ============================================================================

	struct SeparatorStyle {
		// ImGuiStyleVar_SeparatorTextBorderSize
		f32 text_border_size = 0.0f;

		// ImGuiStyleVar_SeparatorTextAlign
		Vec2 text_align{};

		// ImGuiStyleVar_SeparatorTextPadding
		Vec2 text_padding{};

		// ImGuiCol_Separator
		Color color{};

		// ImGuiCol_SeparatorHovered
		Color hovered{};

		// ImGuiCol_SeparatorActive
		Color active{};
	};

	// ============================================================================
	// Tab
	// ============================================================================

	struct TabStyle {
		// ImGuiStyleVar_TabRounding
		f32 rounding = 0.0f;

		// ImGuiStyleVar_TabBorderSize
		f32 border_size = 0.0f;

		// ImGuiStyleVar_TabMinWidthBase
		f32 min_width_base = 0.0f;

		// ImGuiStyleVar_TabMinWidthShrink
		f32 min_width_shrink = 0.0f;

		// ImGuiStyleVar_TabBarBorderSize
		f32 bar_border_size = 0.0f;

		// ImGuiStyleVar_TabBarOverlineSize
		f32 bar_overline_size = 0.0f;

		// ImGuiCol_Tab
		Color bg{};

		// ImGuiCol_TabHovered
		Color bg_hovered{};

		// ImGuiCol_TabSelected
		Color bg_selected{};

		// ImGuiCol_TabSelectedOverline
		Color selected_overline{};

		// ImGuiCol_TabDimmed
		Color bg_dimmed{};

		// ImGuiCol_TabDimmedSelected
		Color bg_dimmed_selected{};

		// ImGuiCol_TabDimmedSelectedOverline
		Color dimmed_selected_overline{};
	};

	// ============================================================================
	// Table
	// ============================================================================

	struct TableStyle {
		// ImGuiStyleVar_CellPadding
		Vec2 cell_padding{};

		// ImGuiStyleVar_TableAngledHeadersAngle
		f32 angled_headers_angle = 0.0f;

		// ImGuiStyleVar_TableAngledHeadersTextAlign
		Vec2 angled_headers_text_align{};

		// ImGuiCol_TableHeaderBg
		Color header_bg{};

		// ImGuiCol_TableBorderStrong
		Color border_strong{};

		// ImGuiCol_TableBorderLight
		Color border_light{};

		// ImGuiCol_TableRowBg
		Color row_bg{};

		// ImGuiCol_TableRowBgAlt
		Color row_bg_alt{};
	};

	// ============================================================================
	// Plot
	// ============================================================================

	struct PlotStyle {
		// ImGuiCol_PlotLines
		Color lines{};

		// ImGuiCol_PlotLinesHovered
		Color lines_hovered{};

		// ImGuiCol_PlotHistogram
		Color histogram{};

		// ImGuiCol_PlotHistogramHovered
		Color histogram_hovered{};
	};

	// ============================================================================
	// Tree
	// ============================================================================

	struct TreeStyle {
		// ImGuiStyleVar_TreeLinesSize
		f32 lines_size = 0.0f;

		// ImGuiStyleVar_TreeLinesRounding
		f32 lines_rounding = 0.0f;

		// ImGuiCol_TreeLines
		Color lines{};
	};

	// ============================================================================
	// Docking
	// ============================================================================

	struct DockingStyle {
		// ImGuiStyleVar_DockingSeparatorSize
		f32 separator_size = 0.0f;

		// ImGuiCol_DockingPreview
		Color preview{};

		// ImGuiCol_DockingEmptyBg
		Color empty_bg{};
	};

	// ============================================================================
	// Navigation / Modal / Overlay
	// ============================================================================

	struct NavigationStyle {
		// ImGuiCol_NavCursor
		Color cursor{};

		// ImGuiCol_NavWindowingHighlight
		Color windowing_highlight{};

		// ImGuiCol_NavWindowingDimBg
		Color windowing_dim_bg{};

		// ImGuiCol_ModalWindowDimBg
		Color modal_dim_bg{};
	};

	// ============================================================================
	// Drag & Drop
	// ============================================================================

	struct DragDropStyle {
		// ImGuiCol_DragDropTarget
		Color target{};
	};

	// ============================================================================
	// Image
	// ============================================================================

	struct ImageStyle {
		// ImGuiStyleVar_ImageBorderSize
		f32 border_size = 0.0f;
	};
	
	
	// ============================================================================
	// Animations
	// ============================================================================
	
	struct TransitionAnimation {
		f32 speed = 16.0f;
		f32 delay = 0.0f;
		Ease ease = Ease::Linear;
	};

	struct HoverAnimation {
		TransitionAnimation transition;
		Vec2 padding = Vec2(3.0f, 3.0f);
		Vec2 offset  = Vec2(0.0f, 0.0f);

		f32 scale   = 1.0f;
		f32 opacity = 1.0f;
	};

	struct PressAnimation {
		TransitionAnimation transition;

		Vec2 padding = Vec2(3.0f, 3.0f);
		Vec2 offset  = Vec2(0.0f, 1.0f);

		f32 scale   = 0.98f;
		f32 opacity = 1.0f;
	};

	struct ColorAnimation {
		TransitionAnimation transition;

		f32 amount = 1.0f;
	};

	struct AlphaAnimation {
		TransitionAnimation transition;

		f32 opacity = 1.0f;
	};

	struct ShapeAnimation {
		TransitionAnimation transition;

		f32 rounding_amount = 1.0f;
		f32 border_amount   = 1.0f;
	};
}// namespace Trinex::UI
