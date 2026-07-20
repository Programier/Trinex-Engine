#include "api_internal.hpp"
#include <Core/etl/variant.hpp>
#include <UI/style.hpp>

namespace Trinex::UI
{
	template<typename T>
	struct OverridedValue {
		T& dst;
		const T& src;
	};

	struct OverridedColor {
		ImGuiCol id;
		const Optional<Color>& value;
	};


	struct OverridedVar {
		ImGuiStyleVar id;
		Variant<const Optional<f32>*, const Optional<Vec2>*> value;

		OverridedVar(ImGuiStyleVar id, const Optional<f32>& value) : id(id), value(&value) {}
		OverridedVar(ImGuiStyleVar id, const Optional<Vec2>& value) : id(id), value(&value) {}
	};

	static i32 push_style_color(const std::initializer_list<OverridedColor>& values)
	{
		i32 result = 0;

		for (const OverridedColor& color : values)
		{
			if (color.value.has_value())
			{
				ImGui::PushStyleColor(color.id, to_imvec(color.value.value()));
				++result;
			}
		}

		return result;
	}

	static void push_style_var_value(ImGuiStyleVar id, f32 value)
	{
		ImGui::PushStyleVar(id, value);
	}

	static void push_style_var_value(ImGuiStyleVar id, const Vec2& value)
	{
		ImGui::PushStyleVar(id, to_imvec(value));
	}

	static i32 push_style_var(const std::initializer_list<OverridedVar>& vars)
	{
		i32 result = 0;

		for (const OverridedVar& var : vars)
		{
			auto visitor = [&](const auto* value) {
				if (value->has_value())
				{
					push_style_var_value(var.id, value->value());
					++result;
				}
			};

			etl::visit(visitor, var.value);
		}

		return result;
	}

	static void pop_basic_style(ContextStack* stack)
	{
		ImGui::PopStyleVar(*stack->pop<u32>());
		ImGui::PopStyleColor(*stack->pop<u32>());
	}

	template<typename T>
	static void pop_overrided_value(ContextStack* stack)
	{
		T* ptr = *stack->pop<T*>();
		(*ptr) = *stack->pop<T>();
	}

	template<typename T, typename... Ts>
	static void pop_overrided_style(ContextStack* stack)
	{
		if constexpr (sizeof...(Ts) > 0)
		{
			pop_overrided_style<Ts...>(stack);
		}

		pop_overrided_value<T>(stack);
	}

	template<typename... Ts>
	static void pop_mixed_style(ContextStack* stack)
	{
		pop_basic_style(stack);
		pop_overrided_style<Ts...>(stack);
	}

	static inline void push_basic_style(u32 colors, u32 vars)
	{
		auto& stack = active_context()->stack;
		stack.push<u32>(colors);
		stack.push<u32>(vars);
		stack.push<void (*)(ContextStack*)>(pop_basic_style);
	}

	template<typename T>
	static inline void push_override_value(ContextStack* stack, const OverridedValue<T>& value)
	{
		stack->push<T>(value.dst);
		stack->push<T*>(&value.dst);
		value.dst = value.src;
	}

	template<typename... Ts>
	static inline void push_override_style(const OverridedValue<Ts>&... values)
	{
		auto& stack = active_context()->stack;
		(push_override_value(&stack, values), ...);
		stack.push<void (*)(ContextStack*)>(pop_overrided_style<Ts...>);
	}

	template<typename... Ts>
	static inline void push_mixed_style(u32 colors, u32 vars, const OverridedValue<Ts>&... values)
	{
		auto& stack = active_context()->stack;
		(push_override_value(&stack, values), ...);

		stack.push<u32>(colors);
		stack.push<u32>(vars);

		stack.push<void (*)(ContextStack*)>(pop_mixed_style<Ts...>);
	}

	void push_style(const GlobalStyle& value)
	{
		const i32 vars = push_style_var({
		        {ImGuiStyleVar_Alpha, value.alpha},
		        {ImGuiStyleVar_DisabledAlpha, value.disabled_alpha},
		});

		push_basic_style(0, vars);
	}

	void push_style(const TextStyle& value)
	{
		const i32 colors = push_style_color({
		        {ImGuiCol_Text, value.color},
		        {ImGuiCol_TextDisabled, value.disabled},
		        {ImGuiCol_TextLink, value.link},
		        {ImGuiCol_TextSelectedBg, value.selection_bg},
		        {ImGuiCol_InputTextCursor, value.cursor},
		});
		push_basic_style(colors, 0);
	}

	void push_style(const WindowStyle& value)
	{
		const i32 colors = push_style_color({
		        {ImGuiCol_WindowBg, value.bg},
		        {ImGuiCol_Border, value.border},
		        {ImGuiCol_BorderShadow, value.border_shadow},
		        {ImGuiCol_TitleBg, value.title_bg},
		        {ImGuiCol_TitleBgActive, value.title_bg_active},
		        {ImGuiCol_TitleBgCollapsed, value.title_bg_collapsed},
		        {ImGuiCol_MenuBarBg, value.menu_bar_bg},
		        {ImGuiCol_ResizeGrip, value.resize_grip},
		        {ImGuiCol_ResizeGripHovered, value.resize_grip_hovered},
		        {ImGuiCol_ResizeGripActive, value.resize_grip_active},
		});

		const i32 vars = push_style_var({
		        {ImGuiStyleVar_WindowPadding, value.padding},
		        {ImGuiStyleVar_WindowMinSize, value.min_size},
		        {ImGuiStyleVar_WindowTitleAlign, value.title_align},
		        {ImGuiStyleVar_WindowRounding, value.rounding},
		        {ImGuiStyleVar_WindowBorderSize, value.border_size},
		});

		push_basic_style(colors, vars);
	}

	void push_style(const ChildStyle& value)
	{
		const i32 colors = push_style_color({{ImGuiCol_ChildBg, value.bg}});

		const i32 vars = push_style_var({
		        {ImGuiStyleVar_ChildRounding, value.rounding},
		        {ImGuiStyleVar_ChildBorderSize, value.border_size},
		});

		push_basic_style(colors, vars);
	}

	void push_style(const PopupStyle& value)
	{
		const i32 colors = push_style_color({{ImGuiCol_PopupBg, value.bg}});

		const i32 vars = push_style_var({
		        {ImGuiStyleVar_PopupRounding, value.rounding},
		        {ImGuiStyleVar_PopupBorderSize, value.border_size},
		});

		push_basic_style(colors, vars);
	}

	void push_style(const LayoutStyle& value)
	{
		const i32 vars = push_style_var({
		        {ImGuiStyleVar_ItemSpacing, value.item_spacing},
		        {ImGuiStyleVar_ItemInnerSpacing, value.item_inner_spacing},
		        {ImGuiStyleVar_IndentSpacing, value.indent_spacing},
		});

		push_basic_style(0, vars);
	}

	void push_style(const FrameStyle& value)
	{
		const i32 colors = push_style_color({
		        {ImGuiCol_FrameBg, value.bg},
		        {ImGuiCol_FrameBgHovered, value.bg_hovered},
		        {ImGuiCol_FrameBgActive, value.bg_active},
		});

		const i32 vars = push_style_var({
		        {ImGuiStyleVar_FramePadding, value.padding},
		        {ImGuiStyleVar_FrameRounding, value.rounding},
		        {ImGuiStyleVar_FrameBorderSize, value.border_size},
		});

		push_basic_style(colors, vars);
	}

	void push_style(const ButtonStyle& value)
	{
		const i32 colors = push_style_color({
		        {ImGuiCol_Button, value.bg},
		        {ImGuiCol_ButtonHovered, value.bg_hovered},
		        {ImGuiCol_ButtonActive, value.bg_active},
		});

		const i32 vars = push_style_var({
		        {ImGuiStyleVar_ButtonTextAlign, value.text_align},
		});

		push_basic_style(colors, vars);
	}

	void push_style(const MarkStyle& value)
	{
		const i32 colors = push_style_color({
		        {ImGuiCol_CheckMark, value.check_mark},
		});
		push_basic_style(colors, 0);
	}

	void push_style(const HeaderStyle& value)
	{
		const i32 colors = push_style_color({
		        {ImGuiCol_Header, value.bg},
		        {ImGuiCol_HeaderHovered, value.bg_hovered},
		        {ImGuiCol_HeaderActive, value.bg_active},
		});

		const i32 vars = push_style_var({{ImGuiStyleVar_SelectableTextAlign, value.selectable_text_align}});
		push_basic_style(colors, vars);
	}

	void push_style(const ScrollbarStyle& value)
	{
		const i32 colors = push_style_color({
		        {ImGuiCol_ScrollbarBg, value.bg},
		        {ImGuiCol_ScrollbarGrab, value.grab},
		        {ImGuiCol_ScrollbarGrabHovered, value.grab_hovered},
		        {ImGuiCol_ScrollbarGrabActive, value.grab_active},
		});

		const i32 vars = push_style_var({
		        {ImGuiStyleVar_ScrollbarSize, value.size},
		        {ImGuiStyleVar_ScrollbarRounding, value.rounding},
		});

		push_basic_style(colors, vars);
	}

	void push_style(const GrabStyle& value)
	{
		const i32 colors = push_style_color({
		        {ImGuiCol_SliderGrab, value.slider_grab},
		        {ImGuiCol_SliderGrabActive, value.slider_grab_active},
		});

		const i32 vars = push_style_var({
		        {ImGuiStyleVar_GrabMinSize, value.min_size},
		        {ImGuiStyleVar_GrabRounding, value.rounding},
		});

		push_basic_style(colors, vars);
	}

	void push_style(const SeparatorStyle& value)
	{
		const i32 colors = push_style_color({
		        {ImGuiCol_Separator, value.color},
		        {ImGuiCol_SeparatorHovered, value.hovered},
		        {ImGuiCol_SeparatorActive, value.active},
		});

		const i32 vars = push_style_var({
		        {ImGuiStyleVar_SeparatorTextBorderSize, value.text_border_size},
		        {ImGuiStyleVar_SeparatorTextAlign, value.text_align},
		        {ImGuiStyleVar_SeparatorTextPadding, value.text_padding},
		});

		push_basic_style(colors, vars);
	}

	void push_style(const TabStyle& value)
	{
		const i32 colors = push_style_color({
		        {ImGuiCol_Tab, value.bg},
		        {ImGuiCol_TabHovered, value.bg_hovered},
		        {ImGuiCol_TabSelected, value.bg_selected},
		        {ImGuiCol_TabSelectedOverline, value.selected_overline},
		        {ImGuiCol_TabDimmed, value.bg_dimmed},
		        {ImGuiCol_TabDimmedSelected, value.bg_dimmed_selected},
		        {ImGuiCol_TabDimmedSelectedOverline, value.dimmed_selected_overline},
		});

		const i32 vars = push_style_var({
		        {ImGuiStyleVar_TabRounding, value.rounding},
		        {ImGuiStyleVar_TabBorderSize, value.border_size},
		        {ImGuiStyleVar_TabMinWidthBase, value.min_width_base},
		        {ImGuiStyleVar_TabMinWidthShrink, value.min_width_shrink},
		        {ImGuiStyleVar_TabBarBorderSize, value.bar_border_size},
		        {ImGuiStyleVar_TabBarOverlineSize, value.bar_overline_size},
		});

		push_basic_style(colors, vars);
	}

	void push_style(const TableStyle& value)
	{
		const i32 colors = push_style_color({
		        {ImGuiCol_TableHeaderBg, value.header_bg},
		        {ImGuiCol_TableBorderStrong, value.border_strong},
		        {ImGuiCol_TableBorderLight, value.border_light},
		        {ImGuiCol_TableRowBg, value.row_bg},
		        {ImGuiCol_TableRowBgAlt, value.row_bg_alt},
		});

		const i32 vars = push_style_var({
		        {ImGuiStyleVar_CellPadding, value.cell_padding},
		        {ImGuiStyleVar_TableAngledHeadersAngle, value.angled_headers_angle},
		        {ImGuiStyleVar_TableAngledHeadersTextAlign, value.angled_headers_text_align},
		});

		push_basic_style(colors, vars);
	}

	void push_style(const PlotStyle& value)
	{
		const i32 colors = push_style_color({
		        {ImGuiCol_PlotLines, value.lines},
		        {ImGuiCol_PlotLinesHovered, value.lines_hovered},
		        {ImGuiCol_PlotHistogram, value.histogram},
		        {ImGuiCol_PlotHistogramHovered, value.histogram_hovered},
		});

		push_basic_style(colors, 0);
	}

	void push_style(const TreeStyle& value)
	{
		const i32 colors = push_style_color({{ImGuiCol_TreeLines, value.lines}});

		const i32 vars = push_style_var({
		        {ImGuiStyleVar_TreeLinesSize, value.lines_size},
		        {ImGuiStyleVar_TreeLinesRounding, value.lines_rounding},
		});

		push_basic_style(colors, vars);
	}

	void push_style(const DockingStyle& value)
	{
		const i32 colors = push_style_color({
		        {ImGuiCol_DockingPreview, value.preview},
		        {ImGuiCol_DockingEmptyBg, value.empty_bg},
		});

		const i32 vars = push_style_var({{ImGuiStyleVar_DockingSeparatorSize, value.separator_size}});
		push_basic_style(colors, vars);
	}

	void push_style(const NavigationStyle& value)
	{
		const i32 colors = push_style_color({
		        {ImGuiCol_NavCursor, value.cursor},
		        {ImGuiCol_NavWindowingHighlight, value.windowing_highlight},
		        {ImGuiCol_NavWindowingDimBg, value.windowing_dim_bg},
		        {ImGuiCol_ModalWindowDimBg, value.modal_dim_bg},
		});

		push_basic_style(colors, 0);
	}

	void push_style(const DragDropStyle& value)
	{
		const i32 colors = push_style_color({{ImGuiCol_DragDropTarget, value.target}});
		push_basic_style(colors, 0);
	}

	void push_style(const ImageStyle& value)
	{
		const i32 vars = push_style_var({{ImGuiStyleVar_ImageBorderSize, value.border_size}});
		push_basic_style(0, vars);
	}

	void pop_style(u32 count)
	{
		auto& stack = active_context()->stack;

		while (count > 0)
		{
			if (auto cmd = *stack.pop<void (*)(ContextStack*)>())
			{
				cmd(&stack);
			}
			--count;
		}
	}
}// namespace Trinex::UI
