#pragma once
#include <Core/etl/optional.hpp>
#include <UI/core.hpp>

namespace Trinex::UI
{
	// ============================================================================
	// Global
	// ============================================================================

	struct GlobalStyle {
		// ImGuiStyleVar_Alpha
		Optional<f32> alpha;

		// ImGuiStyleVar_DisabledAlpha
		Optional<f32> disabled_alpha;
	};

	// ============================================================================
	// Text
	// ============================================================================

	struct TextStyle {
		// ImGuiCol_Text
		Optional<Color> color;

		// ImGuiCol_TextDisabled
		Optional<Color> disabled;

		// ImGuiCol_TextLink
		Optional<Color> link;

		// ImGuiCol_TextSelectedBg
		Optional<Color> selection_bg;

		// ImGuiCol_InputTextCursor
		Optional<Color> cursor;
	};

	// ============================================================================
	// Window
	// ============================================================================

	struct WindowStyle {
		// ImGuiStyleVar_WindowPadding
		Optional<Vec2> padding;

		// ImGuiStyleVar_WindowMinSize
		Optional<Vec2> min_size;

		// ImGuiStyleVar_WindowTitleAlign
		Optional<Vec2> title_align;

		// ImGuiStyleVar_WindowRounding
		Optional<f32> rounding;

		// ImGuiStyleVar_WindowBorderSize
		Optional<f32> border_size;

		// ImGuiCol_WindowBg
		Optional<Color> bg;

		// ImGuiCol_Border
		Optional<Color> border;

		// ImGuiCol_BorderShadow
		Optional<Color> border_shadow;

		// ImGuiCol_TitleBg
		Optional<Color> title_bg;

		// ImGuiCol_TitleBgActive
		Optional<Color> title_bg_active;

		// ImGuiCol_TitleBgCollapsed
		Optional<Color> title_bg_collapsed;

		// ImGuiCol_MenuBarBg
		Optional<Color> menu_bar_bg;

		// ImGuiCol_ResizeGrip
		Optional<Color> resize_grip;

		// ImGuiCol_ResizeGripHovered
		Optional<Color> resize_grip_hovered;

		// ImGuiCol_ResizeGripActive
		Optional<Color> resize_grip_active;
	};

	// ============================================================================
	// Child Window
	// ============================================================================

	struct ChildStyle {
		// ImGuiStyleVar_ChildRounding
		Optional<f32> rounding;

		// ImGuiStyleVar_ChildBorderSize
		Optional<f32> border_size;

		// ImGuiCol_ChildBg
		Optional<Color> bg;
	};

	// ============================================================================
	// Popup / Tooltip / Menu Popup
	// ============================================================================

	struct PopupStyle {
		// ImGuiStyleVar_PopupRounding
		Optional<f32> rounding;

		// ImGuiStyleVar_PopupBorderSize
		Optional<f32> border_size;

		// Logical opacity for PopupBg alpha.
		Optional<f32> opacity;

		// ImGuiCol_PopupBg
		Optional<Color> bg;
	};

	// ============================================================================
	// Layout
	// ============================================================================

	struct LayoutStyle {
		// ImGuiStyleVar_ItemSpacing
		Optional<Vec2> item_spacing;

		// ImGuiStyleVar_ItemInnerSpacing
		Optional<Vec2> item_inner_spacing;

		// ImGuiStyleVar_IndentSpacing
		Optional<f32> indent_spacing;
	};

	// ============================================================================
	// Frame
	// ============================================================================

	struct FrameStyle {
		// ImGuiStyleVar_FramePadding
		Optional<Vec2> padding;

		// ImGuiStyleVar_FrameRounding
		Optional<f32> rounding = 8.0f;

		// ImGuiStyleVar_FrameBorderSize
		Optional<f32> border_size = 1.0f;

		// ImGuiCol_FrameBg
		Optional<Color> bg;

		// ImGuiCol_FrameBgHovered
		Optional<Color> bg_hovered;

		// ImGuiCol_FrameBgActive
		Optional<Color> bg_active;
	};

	// ============================================================================
	// Button
	// ============================================================================

	struct ButtonStyle {
		// ImGuiStyleVar_ButtonTextAlign
		Optional<Vec2> text_align;

		// ImGuiCol_Button
		Optional<Color> bg;

		// ImGuiCol_ButtonHovered
		Optional<Color> bg_hovered;

		// ImGuiCol_ButtonActive
		Optional<Color> bg_active;
	};

	// ============================================================================
	// Checkbox / Radio / Markers
	// ============================================================================

	struct MarkStyle {
		// ImGuiCol_CheckMark
		Optional<Color> check_mark;
	};

	// ============================================================================
	// Header / Selectable / TreeNode / MenuItem
	// ============================================================================

	struct HeaderStyle {
		// ImGuiStyleVar_SelectableTextAlign
		Optional<Vec2> selectable_text_align;

		// ImGuiCol_Header
		Optional<Color> bg;

		// ImGuiCol_HeaderHovered
		Optional<Color> bg_hovered;

		// ImGuiCol_HeaderActive
		Optional<Color> bg_active;
	};

	// ============================================================================
	// Scrollbar
	// ============================================================================

	struct ScrollbarStyle {
		// ImGuiStyleVar_ScrollbarSize
		Optional<f32> size;

		// ImGuiStyleVar_ScrollbarRounding
		Optional<f32> rounding;

		// ImGuiCol_ScrollbarBg
		Optional<Color> bg;

		// ImGuiCol_ScrollbarGrab
		Optional<Color> grab;

		// ImGuiCol_ScrollbarGrabHovered
		Optional<Color> grab_hovered;

		// ImGuiCol_ScrollbarGrabActive
		Optional<Color> grab_active;
	};

	// ============================================================================
	// Slider / Drag Grab
	// ============================================================================

	struct GrabStyle {
		// ImGuiStyleVar_GrabMinSize
		Optional<f32> min_size = 12.0f;

		// ImGuiStyleVar_GrabRounding
		Optional<f32> rounding = 8.0f;

		// ImGuiCol_SliderGrab
		Optional<Color> slider_grab;

		// ImGuiCol_SliderGrabActive
		Optional<Color> slider_grab_active;
	};

	// ============================================================================
	// Separator
	// ============================================================================

	struct SeparatorStyle {
		// ImGuiStyleVar_SeparatorTextBorderSize
		Optional<f32> text_border_size = 1.0f;

		// ImGuiStyleVar_SeparatorTextAlign
		Optional<Vec2> text_align;

		// ImGuiStyleVar_SeparatorTextPadding
		Optional<Vec2> text_padding;

		// ImGuiCol_Separator
		Optional<Color> color;

		// ImGuiCol_SeparatorHovered
		Optional<Color> hovered;

		// ImGuiCol_SeparatorActive
		Optional<Color> active;
	};

	// ============================================================================
	// Tab
	// ============================================================================

	struct TabStyle {
		// ImGuiStyleVar_TabRounding
		Optional<f32> rounding;

		// ImGuiStyleVar_TabBorderSize
		Optional<f32> border_size;

		// ImGuiStyleVar_TabMinWidthBase
		Optional<f32> min_width_base;

		// ImGuiStyleVar_TabMinWidthShrink
		Optional<f32> min_width_shrink;

		// ImGuiStyleVar_TabBarBorderSize
		Optional<f32> bar_border_size;

		// ImGuiStyleVar_TabBarOverlineSize
		Optional<f32> bar_overline_size;

		// ImGuiCol_Tab
		Optional<Color> bg;

		// ImGuiCol_TabHovered
		Optional<Color> bg_hovered;

		// ImGuiCol_TabSelected
		Optional<Color> bg_selected;

		// ImGuiCol_TabSelectedOverline
		Optional<Color> selected_overline;

		// ImGuiCol_TabDimmed
		Optional<Color> bg_dimmed;

		// ImGuiCol_TabDimmedSelected
		Optional<Color> bg_dimmed_selected;

		// ImGuiCol_TabDimmedSelectedOverline
		Optional<Color> dimmed_selected_overline;
	};

	// ============================================================================
	// Table
	// ============================================================================

	struct TableStyle {
		// ImGuiStyleVar_CellPadding
		Optional<Vec2> cell_padding;

		// ImGuiStyleVar_TableAngledHeadersAngle
		Optional<f32> angled_headers_angle;

		// ImGuiStyleVar_TableAngledHeadersTextAlign
		Optional<Vec2> angled_headers_text_align;

		// ImGuiCol_TableHeaderBg
		Optional<Color> header_bg;

		// ImGuiCol_TableBorderStrong
		Optional<Color> border_strong;

		// ImGuiCol_TableBorderLight
		Optional<Color> border_light;

		// ImGuiCol_TableRowBg
		Optional<Color> row_bg;

		// ImGuiCol_TableRowBgAlt
		Optional<Color> row_bg_alt;
	};

	// ============================================================================
	// Plot
	// ============================================================================

	struct PlotStyle {
		// ImGuiCol_PlotLines
		Optional<Color> lines;

		// ImGuiCol_PlotLinesHovered
		Optional<Color> lines_hovered;

		// ImGuiCol_PlotHistogram
		Optional<Color> histogram;

		// ImGuiCol_PlotHistogramHovered
		Optional<Color> histogram_hovered;
	};

	// ============================================================================
	// Tree
	// ============================================================================

	struct TreeStyle {
		// ImGuiStyleVar_TreeLinesSize
		Optional<f32> lines_size;

		// ImGuiStyleVar_TreeLinesRounding
		Optional<f32> lines_rounding;

		// ImGuiCol_TreeLines
		Optional<Color> lines;
	};

	// ============================================================================
	// Docking
	// ============================================================================

	struct DockingStyle {
		// ImGuiStyleVar_DockingSeparatorSize
		Optional<f32> separator_size;

		// ImGuiCol_DockingPreview
		Optional<Color> preview;

		// ImGuiCol_DockingEmptyBg
		Optional<Color> empty_bg;
	};

	// ============================================================================
	// Navigation / Modal / Overlay
	// ============================================================================

	struct NavigationStyle {
		// ImGuiCol_NavCursor
		Optional<Color> cursor;

		// ImGuiCol_NavWindowingHighlight
		Optional<Color> windowing_highlight;

		// ImGuiCol_NavWindowingDimBg
		Optional<Color> windowing_dim_bg;

		// ImGuiCol_ModalWindowDimBg
		Optional<Color> modal_dim_bg;
	};

	// ============================================================================
	// Drag & Drop
	// ============================================================================

	struct DragDropStyle {
		// ImGuiCol_DragDropTarget
		Optional<Color> target;
	};

	// ============================================================================
	// Image
	// ============================================================================

	struct ImageStyle {
		// ImGuiStyleVar_ImageBorderSize
		Optional<f32> border_size;
	};


	// ============================================================================
	// Animations
	// ============================================================================

	struct TransitionAnimation {
		Optional<f32> speed;
		Optional<f32> delay;
		Optional<Ease> ease;
	};

	struct HoverAnimation {
		TransitionAnimation transition;
		Optional<Vec2> padding;
		Optional<Vec2> offset;

		Optional<f32> scale;
		Optional<f32> opacity;
	};

	struct PressAnimation {
		TransitionAnimation transition;

		Optional<Vec2> padding;
		Optional<Vec2> offset;

		Optional<f32> scale;
		Optional<f32> opacity;
	};

	struct ColorAnimation {
		TransitionAnimation transition;

		Optional<f32> amount;
	};

	struct AlphaAnimation {
		TransitionAnimation transition;

		Optional<f32> opacity;
	};

	struct ShapeAnimation {
		TransitionAnimation transition;

		Optional<f32> rounding_amount;
		Optional<f32> border_amount;
	};
}// namespace Trinex::UI
