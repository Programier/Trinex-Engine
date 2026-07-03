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
		f32 disabled_alpha = 0.45f;
	};

	// ============================================================================
	// Text
	// ============================================================================

	struct TextStyle {
		// ImGuiCol_Text
		Color color = Color(0.92f, 0.95f, 0.98f, 1.00f);

		// ImGuiCol_TextDisabled
		Color disabled = Color(0.45f, 0.50f, 0.60f, 1.00f);

		// ImGuiCol_TextLink
		Color link = Color(0.40f, 0.91f, 0.98f, 1.00f);

		// ImGuiCol_TextSelectedBg
		Color selection_bg = Color(0.32f, 0.72f, 0.95f, 0.35f);

		// ImGuiCol_InputTextCursor
		Color cursor = Color(0.67f, 0.97f, 1.00f, 1.00f);
	};

	// ============================================================================
	// Window
	// ============================================================================

	struct WindowStyle {
		// ImGuiStyleVar_WindowPadding
		Vec2 padding{14.0f, 12.0f};

		// ImGuiStyleVar_WindowMinSize
		Vec2 min_size{64.0f, 64.0f};

		// ImGuiStyleVar_WindowTitleAlign
		Vec2 title_align{0.5f, 0.5f};

		// ImGuiStyleVar_WindowRounding
		f32 rounding = 12.0f;

		// ImGuiStyleVar_WindowBorderSize
		f32 border_size = 1.0f;

		// Logical opacity for WindowBg alpha.
		f32 opacity = 0.98f;

		// ImGuiCol_WindowBg
		Color bg = Color(0.045f, 0.065f, 0.120f, 0.98f);

		// ImGuiCol_Border
		Color border = Color(0.23f, 0.31f, 0.48f, 0.65f);

		// ImGuiCol_BorderShadow
		Color border_shadow = Color(0.00f, 0.00f, 0.00f, 0.35f);

		// ImGuiCol_TitleBg
		Color title_bg = Color(0.055f, 0.080f, 0.150f, 1.00f);

		// ImGuiCol_TitleBgActive
		Color title_bg_active = Color(0.10f, 0.16f, 0.28f, 1.00f);

		// ImGuiCol_TitleBgCollapsed
		Color title_bg_collapsed = Color(0.035f, 0.050f, 0.095f, 0.90f);

		// ImGuiCol_MenuBarBg
		Color menu_bar_bg = Color(0.065f, 0.095f, 0.170f, 1.00f);

		// ImGuiCol_ResizeGrip
		Color resize_grip = Color(0.40f, 0.91f, 0.98f, 0.28f);

		// ImGuiCol_ResizeGripHovered
		Color resize_grip_hovered = Color(0.40f, 0.91f, 0.98f, 0.55f);

		// ImGuiCol_ResizeGripActive
		Color resize_grip_active = Color(0.65f, 0.55f, 0.98f, 0.85f);
	};

	// ============================================================================
	// Child Window
	// ============================================================================

	struct ChildStyle {
		// ImGuiStyleVar_ChildRounding
		f32 rounding = 10.0f;

		// ImGuiStyleVar_ChildBorderSize
		f32 border_size = 1.0f;

		// Logical opacity for ChildBg alpha.
		f32 opacity = 0.92f;

		// ImGuiCol_ChildBg
		Color bg = Color(0.065f, 0.090f, 0.155f, 0.92f);
	};

	// ============================================================================
	// Popup / Tooltip / Menu Popup
	// ============================================================================

	struct PopupStyle {
		// ImGuiStyleVar_PopupRounding
		f32 rounding = 10.0f;

		// ImGuiStyleVar_PopupBorderSize
		f32 border_size = 1.0f;

		// Logical opacity for PopupBg alpha.
		f32 opacity = 0.98f;

		// ImGuiCol_PopupBg
		Color bg = Color(0.055f, 0.080f, 0.145f, 0.98f);
	};

	// ============================================================================
	// Layout
	// ============================================================================

	struct LayoutStyle {
		// ImGuiStyleVar_ItemSpacing
		Vec2 item_spacing{10.0f, 8.0f};

		// ImGuiStyleVar_ItemInnerSpacing
		Vec2 item_inner_spacing{8.0f, 6.0f};

		// ImGuiStyleVar_IndentSpacing
		f32 indent_spacing = 22.0f;
	};

	// ============================================================================
	// Frame
	// ============================================================================

	struct FrameStyle {
		// ImGuiStyleVar_FramePadding
		Vec2 padding{10.0f, 6.0f};

		// ImGuiStyleVar_FrameRounding
		f32 rounding = 8.0f;

		// ImGuiStyleVar_FrameBorderSize
		f32 border_size = 1.0f;

		// ImGuiCol_FrameBg
		Color bg = Color(0.085f, 0.120f, 0.200f, 1.00f);

		// ImGuiCol_FrameBgHovered
		Color bg_hovered = Color(0.12f, 0.18f, 0.30f, 1.00f);

		// ImGuiCol_FrameBgActive
		Color bg_active = Color(0.16f, 0.24f, 0.40f, 1.00f);
	};

	// ============================================================================
	// Button
	// ============================================================================

	struct ButtonStyle {
		// ImGuiStyleVar_ButtonTextAlign
		Vec2 text_align{0.5f, 0.5f};

		// ImGuiCol_Button
		Color bg = Color(0.13f, 0.23f, 0.36f, 1.00f);

		// ImGuiCol_ButtonHovered
		Color bg_hovered = Color(0.18f, 0.35f, 0.52f, 1.00f);

		// ImGuiCol_ButtonActive
		Color bg_active = Color(0.40f, 0.91f, 0.98f, 0.80f);
	};

	// ============================================================================
	// Checkbox / Radio / Markers
	// ============================================================================

	struct MarkStyle {
		// ImGuiCol_CheckMark
		Color check_mark = Color(0.67f, 0.97f, 1.00f, 1.00f);
	};

	// ============================================================================
	// Header / Selectable / TreeNode / MenuItem
	// ============================================================================

	struct HeaderStyle {
		// ImGuiStyleVar_SelectableTextAlign
		Vec2 selectable_text_align{0.0f, 0.5f};

		// ImGuiCol_Header
		Color bg = Color(0.12f, 0.18f, 0.30f, 0.85f);

		// ImGuiCol_HeaderHovered
		Color bg_hovered = Color(0.18f, 0.35f, 0.52f, 0.95f);

		// ImGuiCol_HeaderActive
		Color bg_active = Color(0.40f, 0.91f, 0.98f, 0.32f);
	};

	// ============================================================================
	// Scrollbar
	// ============================================================================

	struct ScrollbarStyle {
		// ImGuiStyleVar_ScrollbarSize
		f32 size = 14.0f;

		// ImGuiStyleVar_ScrollbarRounding
		f32 rounding = 10.0f;

		// ImGuiCol_ScrollbarBg
		Color bg = Color(0.035f, 0.050f, 0.095f, 0.65f);

		// ImGuiCol_ScrollbarGrab
		Color grab = Color(0.20f, 0.28f, 0.43f, 0.90f);

		// ImGuiCol_ScrollbarGrabHovered
		Color grab_hovered = Color(0.30f, 0.43f, 0.62f, 0.95f);

		// ImGuiCol_ScrollbarGrabActive
		Color grab_active = Color(0.40f, 0.91f, 0.98f, 0.85f);
	};

	// ============================================================================
	// Slider / Drag Grab
	// ============================================================================

	struct GrabStyle {
		// ImGuiStyleVar_GrabMinSize
		f32 min_size = 12.0f;

		// ImGuiStyleVar_GrabRounding
		f32 rounding = 8.0f;

		// ImGuiCol_SliderGrab
		Color slider_grab = Color(0.40f, 0.91f, 0.98f, 0.85f);

		// ImGuiCol_SliderGrabActive
		Color slider_grab_active = Color(0.65f, 0.55f, 0.98f, 1.00f);
	};

	// ============================================================================
	// Separator
	// ============================================================================

	struct SeparatorStyle {
		// ImGuiStyleVar_SeparatorTextBorderSize
		f32 text_border_size = 1.0f;

		// ImGuiStyleVar_SeparatorTextAlign
		Vec2 text_align{0.0f, 0.5f};

		// ImGuiStyleVar_SeparatorTextPadding
		Vec2 text_padding{20.0f, 4.0f};

		// ImGuiCol_Separator
		Color color = Color(0.23f, 0.31f, 0.48f, 0.65f);

		// ImGuiCol_SeparatorHovered
		Color hovered = Color(0.40f, 0.91f, 0.98f, 0.70f);

		// ImGuiCol_SeparatorActive
		Color active = Color(0.65f, 0.55f, 0.98f, 0.95f);
	};

	// ============================================================================
	// Tab
	// ============================================================================

	struct TabStyle {
		// ImGuiStyleVar_TabRounding
		f32 rounding = 9.0f;

		// ImGuiStyleVar_TabBorderSize
		f32 border_size = 0.0f;

		// ImGuiStyleVar_TabMinWidthBase
		f32 min_width_base = 44.0f;

		// ImGuiStyleVar_TabMinWidthShrink
		f32 min_width_shrink = 24.0f;

		// ImGuiStyleVar_TabBarBorderSize
		f32 bar_border_size = 1.0f;

		// ImGuiStyleVar_TabBarOverlineSize
		f32 bar_overline_size = 2.0f;

		// ImGuiCol_Tab
		Color bg = Color(0.08f, 0.12f, 0.21f, 1.00f);

		// ImGuiCol_TabHovered
		Color bg_hovered = Color(0.18f, 0.35f, 0.52f, 1.00f);

		// ImGuiCol_TabSelected
		Color bg_selected = Color(0.13f, 0.23f, 0.36f, 1.00f);

		// ImGuiCol_TabSelectedOverline
		Color selected_overline = Color(0.40f, 0.91f, 0.98f, 1.00f);

		// ImGuiCol_TabDimmed
		Color bg_dimmed = Color(0.055f, 0.080f, 0.145f, 1.00f);

		// ImGuiCol_TabDimmedSelected
		Color bg_dimmed_selected = Color(0.10f, 0.16f, 0.28f, 1.00f);

		// ImGuiCol_TabDimmedSelectedOverline
		Color dimmed_selected_overline = Color(0.65f, 0.55f, 0.98f, 0.75f);
	};

	// ============================================================================
	// Table
	// ============================================================================

	struct TableStyle {
		// ImGuiStyleVar_CellPadding
		Vec2 cell_padding{10.0f, 7.0f};

		// ImGuiStyleVar_TableAngledHeadersAngle
		f32 angled_headers_angle = 35.0f;

		// ImGuiStyleVar_TableAngledHeadersTextAlign
		Vec2 angled_headers_text_align{0.5f, 0.0f};

		// ImGuiCol_TableHeaderBg
		Color header_bg = Color(0.10f, 0.16f, 0.28f, 1.00f);

		// ImGuiCol_TableBorderStrong
		Color border_strong = Color(0.28f, 0.38f, 0.58f, 0.80f);

		// ImGuiCol_TableBorderLight
		Color border_light = Color(0.20f, 0.27f, 0.42f, 0.55f);

		// ImGuiCol_TableRowBg
		Color row_bg = Color(0.00f, 0.00f, 0.00f, 0.00f);

		// ImGuiCol_TableRowBgAlt
		Color row_bg_alt = Color(0.12f, 0.18f, 0.30f, 0.28f);
	};

	// ============================================================================
	// Plot
	// ============================================================================

	struct PlotStyle {
		// ImGuiCol_PlotLines
		Color lines = Color(0.40f, 0.91f, 0.98f, 1.00f);

		// ImGuiCol_PlotLinesHovered
		Color lines_hovered = Color(0.67f, 0.97f, 1.00f, 1.00f);

		// ImGuiCol_PlotHistogram
		Color histogram = Color(0.65f, 0.55f, 0.98f, 1.00f);

		// ImGuiCol_PlotHistogramHovered
		Color histogram_hovered = Color(0.93f, 0.55f, 0.82f, 1.00f);
	};

	// ============================================================================
	// Tree
	// ============================================================================

	struct TreeStyle {
		// ImGuiStyleVar_TreeLinesSize
		f32 lines_size = 1.5f;

		// ImGuiStyleVar_TreeLinesRounding
		f32 lines_rounding = 4.0f;

		// ImGuiCol_TreeLines
		Color lines = Color(0.40f, 0.91f, 0.98f, 0.45f);
	};

	// ============================================================================
	// Docking
	// ============================================================================

	struct DockingStyle {
		// ImGuiStyleVar_DockingSeparatorSize
		f32 separator_size = 2.0f;

		// ImGuiCol_DockingPreview
		Color preview = Color(0.40f, 0.91f, 0.98f, 0.38f);

		// ImGuiCol_DockingEmptyBg
		Color empty_bg = Color(0.030f, 0.040f, 0.075f, 1.00f);
	};

	// ============================================================================
	// Navigation / Modal / Overlay
	// ============================================================================

	struct NavigationStyle {
		// ImGuiCol_NavCursor
		Color cursor = Color(0.67f, 0.97f, 1.00f, 1.00f);

		// ImGuiCol_NavWindowingHighlight
		Color windowing_highlight = Color(0.67f, 0.97f, 1.00f, 0.70f);

		// ImGuiCol_NavWindowingDimBg
		Color windowing_dim_bg = Color(0.00f, 0.00f, 0.00f, 0.55f);

		// ImGuiCol_ModalWindowDimBg
		Color modal_dim_bg = Color(0.00f, 0.00f, 0.00f, 0.62f);
	};

	// ============================================================================
	// Drag & Drop
	// ============================================================================

	struct DragDropStyle {
		// ImGuiCol_DragDropTarget
		Color target = Color(0.65f, 0.55f, 0.98f, 0.95f);
	};

	// ============================================================================
	// Image
	// ============================================================================

	struct ImageStyle {
		// ImGuiStyleVar_ImageBorderSize
		f32 border_size = 1.0f;
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
