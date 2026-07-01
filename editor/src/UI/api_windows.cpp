#include "api_internal.hpp"
#include <Core/math/math.hpp>
#include <imgui_stacklayout.h>

namespace Trinex::UI
{
	/////////////////////// WINDOWS AND CONTAINERS ///////////////////////

	bool begin_window(StringView name, bool* open, const WindowOptions& options)
	{
		trinex_assert(has_text(name) && "UI::begin_window() requires a non-empty name");
		if (!has_text(name))
		{
			return false;
		}

		if (open != nullptr && *open == false)
		{
			return false;
		}

		apply_window_options_pre_begin(options);
		push_window_styles(false);

		String name_storage(name);
		const bool visible = ImGui::Begin(name_storage.c_str(), open, to_imgui_window_flags(options.flags));
		if (visible)
		{
			apply_window_options_post_begin(options);
		}
		else
		{
			end_window();
		}
		return visible;
	}

	void end_window()
	{
		ImGui::End();
		pop_window_styles();
	}

	void register_widget(Context* context, Widget* widget)
	{
		if (context == nullptr || widget == nullptr)
		{
			return;
		}

		if (PersistentWindow* window = find_window(context, widget))
		{
			return;
		}

		PersistentWindow* window = trx_new PersistentWindow();
		window->next             = context->window_list;
		window->widget           = widget;
		context->window_list     = window;

		widget->on_attach(context);
	}

	void unregister_widget(Context* context, Widget* widget)
	{
		if (context == nullptr || widget == nullptr)
		{
			return;
		}

		PersistentWindow** current = &context->window_list;

		while (*current)
		{
			PersistentWindow* window = *current;

			if (window->widget != widget)
			{
				current = &window->next;
				continue;
			}

			*current = window->next;
			widget->on_deattach(context);
			trx_delete window;
			return;
		}
	}

	bool begin_panel(StringView id, const PanelOptions& options)
	{
		const Vec4 bg = has_color(options.background_color) ? options.background_color : active_context()->style.colors.panel;
		const float rounding = options.rounding >= 0.0f ? options.rounding : active_context()->style.rounding;
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, rounding);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
		                    ImVec2(active_context()->style.padding, active_context()->style.padding));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
		String id_storage(id);
		const bool visible = ImGui::BeginChild(id_storage.c_str(), to_imvec(resolve(options.size)),
		                                       options.flags | ImGuiChildFlags_AlwaysUseWindowPadding, options.window_flags);
		if (!visible)
		{
			ImGui::EndChild();
			ImGui::PopStyleColor(2);
			ImGui::PopStyleVar(3);
			return false;
		}

		PanelContext context;
		context.border           = options.border;
		context.background       = options.background;
		context.rounding         = rounding;
		context.background_color = bg;
		context.border_color     = active_context()->style.colors.border;
		context.draw_shadow      = options.background && has_shadow_override() && shadow_visible(current_shadow());
		context.shadow           = current_shadow();
		active_context()->panel_stack.push_back(context);
		ImGui::GetWindowDrawList()->ChannelsSplit(2);
		ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);
		return true;
	}

	void end_panel()
	{
		auto& stack = active_context()->panel_stack;

		trinex_assert(!stack.empty() && "UI::end_panel() called without matching begin_panel()");
		if (stack.empty())
		{
			return;
		}

		const PanelContext context = stack.back();
		stack.pop_back();

		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->ChannelsSetCurrent(0);
		const ImVec2 min = ImGui::GetWindowPos();
		const ImVec2 max = add(min, ImGui::GetWindowSize());
		if (context.draw_shadow)
		{
			draw_shadow_rect(draw, min, max, context.rounding, context.shadow);
		}
		if (context.background)
		{
			draw->AddRectFilled(min, max, col_u32(context.background_color), context.rounding);
		}
		if (context.border)
		{
			draw->AddRect(min, max, col_u32(context.border_color), context.rounding, 0, active_context()->style.border_size);
		}
		draw->ChannelsSetCurrent(1);
		draw->ChannelsMerge();

		ImGui::EndChild();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(3);
	}

	bool begin_glass_panel(StringView id, Size size, const GlassOptions& options)
	{
		trinex_assert(has_text(id) && "UI::begin_glass_panel() requires a non-empty id");
		if (!has_text(id))
		{
			return false;
		}

		const float rounding = options.rounding >= 0.0f ? options.rounding : active_context()->style.rounding;
		const float padding  = options.padding >= 0.0f ? options.padding : active_context()->style.padding;
		const float opacity  = Math::clamp(options.opacity, 0.0f, 1.0f);

		Vec2 resolved_size = resolve(size);
		if (resolved_size.x <= 0.0f)
		{
			resolved_size.x = ImGui::GetContentRegionAvail().x;
		}

		ImGuiChildFlags child_flags = ImGuiChildFlags_AlwaysUseWindowPadding;
		if (resolved_size.y <= 0.0f)
		{
			child_flags |= ImGuiChildFlags_AutoResizeY;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, rounding);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

		String id_storage(id);
		const bool visible = ImGui::BeginChild(id_storage.c_str(), to_imvec(resolved_size), child_flags, ImGuiWindowFlags_None);
		if (!visible)
		{
			ImGui::EndChild();
			ImGui::PopStyleColor(2);
			ImGui::PopStyleVar(3);
			return false;
		}

		GlassPanelContext context;
		context.id            = ImGui::GetID("glass_panel_anim");
		context.border        = options.border;
		context.background    = options.background;
		context.draw_shadow   = shadow_visible(current_shadow()) && opacity > 0.0f;
		context.highlight_top = options.highlight_top;
		context.rounding      = rounding;
		context.tint          = options.tint;
		context.tint.w *= opacity;
		context.border_color = has_color(options.border_color) ? options.border_color : default_glass_border();
		context.border_color.w *= opacity;
		context.highlight = options.highlight;
		context.highlight.w *= opacity;
		context.shadow = current_shadow();
		context.shadow.color.w *= opacity;

		active_context()->glass_panel_stack.push_back(context);
		ImGui::GetWindowDrawList()->ChannelsSplit(2);
		ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);
		return true;
	}

	void end_glass_panel()
	{
		auto& stack = active_context()->glass_panel_stack;
		trinex_assert(!stack.empty() && "UI::end_glass_panel() called without matching begin_glass_panel()");

		if (stack.empty())
		{
			return;
		}

		const GlassPanelContext context = stack.back();
		stack.pop_back();

		const ImVec2 min = ImGui::GetWindowPos();
		const ImVec2 max = add(min, ImGui::GetWindowSize());
		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->ChannelsSetCurrent(0);

		if (context.draw_shadow)
		{
			draw_shadow_rect(draw, min, max, context.rounding, context.shadow);
		}

		if (context.background && context.tint.w > 0.0f)
		{
			draw->AddRectFilled(min, max, col_u32(context.tint), context.rounding);
		}

		if (context.border)
		{
			draw->AddRect(min, max, col_u32(context.border_color), context.rounding, 0, active_context()->style.border_size);
		}

		if (context.highlight_top && context.highlight.w > 0.0f)
		{
			const float inset = Math::max(1.0f, active_context()->style.border_size);
			const ImVec2 line_min(min.x + context.rounding * 0.35f, min.y + inset);
			const ImVec2 line_max(max.x - context.rounding * 0.35f, min.y + inset);
			draw->AddLine(line_min, line_max, col_u32(context.highlight), 1.0f);
		}
		draw->ChannelsSetCurrent(1);
		draw->ChannelsMerge();
		ImGui::EndChild();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(3);
	}

	bool begin_group_panel(StringView label, const PanelOptions& options)
	{
		ImGui::BeginGroup();
		if (has_text(label))
		{
			text_muted(label);
		}

		if (!begin_panel(label, options))
		{
			ImGui::EndGroup();
			return false;
		}

		return true;
	}

	void end_group_panel()
	{
		end_panel();
		ImGui::EndGroup();
	}

	bool begin_group()
	{
		ImGui::BeginGroup();
		return true;
	}

	void end_group()
	{
		ImGui::EndGroup();
	}

	bool begin_card(StringView title, const CardOptions& options)
	{
		cleanup_states();
		if (has_text(title))
		{
			imgui_push_id(title);
		}
		else
		{
			ImGui::PushID(ImGui::GetID("card_scope"));
		}

		const float rounding = options.rounding >= 0.0f ? options.rounding : active_context()->style.rounding;
		const float padding  = options.padding >= 0.0f ? options.padding : active_context()->style.padding;
		const float spacing  = options.spacing >= 0.0f ? options.spacing : active_context()->style.spacing;
		const Vec4 accent    = has_color(options.accent) ? options.accent : active_context()->style.colors.accent;
		const Vec4 bg     = has_color(options.background_color) ? options.background_color : active_context()->style.colors.panel;
		const Vec4 border = has_color(options.border_color) ? options.border_color : active_context()->style.colors.border;

		Vec2 size = resolve(options.size);
		if (size.x <= 0.0f)
		{
			size.x = ImGui::GetContentRegionAvail().x;
		}

		ImGuiChildFlags child_flags = ImGuiChildFlags_AlwaysUseWindowPadding;
		if (size.y <= 0.0f)
		{
			child_flags |= ImGuiChildFlags_AutoResizeY;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, rounding);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, spacing));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

		const bool visible = ImGui::BeginChild("##card", to_imvec(size), child_flags, ImGuiWindowFlags_None);
		if (!visible)
		{
			ImGui::EndChild();
			ImGui::PopStyleColor(2);
			ImGui::PopStyleVar(3);
			ImGui::PopID();
			return false;
		}

		CardContext context;
		context.id               = ImGui::GetID("card_anim");
		context.disabled         = options.disabled;
		context.hoverable        = options.hoverable;
		context.selected         = options.selected;
		context.border           = options.border;
		context.background       = options.background;
		context.rounding         = rounding;
		context.elevation        = Math::max(0.0f, options.elevation);
		context.accent           = accent;
		context.background_color = bg;
		context.border_color     = border;
		context.shadow           = current_shadow();
		active_context()->card_stack.push_back(context);
		ImGui::GetWindowDrawList()->ChannelsSplit(2);
		ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);

		if (options.disabled)
		{
			begin_disabled(true);
		}
		return true;
	}

	void end_card()
	{
		auto& stack = active_context()->card_stack;
		trinex_assert(!stack.empty() && "UI::end_card() called without matching begin_card()");

		if (stack.empty())
		{
			return;
		}

		const CardContext context = stack.back();
		stack.pop_back();

		const ImVec2 min   = ImGui::GetWindowPos();
		const ImVec2 size  = ImGui::GetWindowSize();
		const ImVec2 max   = add(min, size);
		const bool hovered = context.hoverable && !context.disabled && ImGui::IsMouseHoveringRect(min, max, true);
		AnimState& anim    = state_for(context.id);
		anim.hover         = approach(anim.hover, hovered ? 1.0f : 0.0f);
		anim.selected      = approach(anim.selected, context.selected ? 1.0f : 0.0f);

		const Vec4 accent  = context.accent;
		Vec4 border_target = context.selected ? Math::lerp(context.border_color, accent, 0.7f) : context.border_color;
		Vec4 background    = context.background ? context.background_color : Vec4(0, 0, 0, 0);
		if (context.background)
		{
			background = Math::lerp(background, active_context()->style.colors.background_hovered, anim.hover * 0.40f);
			background = Math::lerp(background, with_alpha(accent, 1.0f), anim.selected * 0.12f);
		}
		border_target = Math::lerp(border_target, accent, anim.hover * 0.30f);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->ChannelsSetCurrent(0);

		if (context.elevation > 0.0f)
		{
			draw_shadow_rect(draw, min, max, context.rounding, scaled_shadow(context.shadow, context.elevation));
		}
		if (context.background)
		{
			draw->AddRectFilled(min, max, col_u32(background), context.rounding);
		}
		if (anim.selected > 0.01f)
		{
			draw->AddRectFilled(min, ImVec2(min.x + 3.0f, max.y), col_u32(accent, 0.95f * anim.selected), context.rounding,
			                    ImDrawFlags_RoundCornersLeft);
		}
		if (context.border)
		{
			const float alpha = context.selected ? 1.0f : Math::lerp(0.85f, 1.0f, anim.hover * 0.65f);
			draw->AddRect(min, max, col_u32(border_target, alpha), context.rounding, 0, active_context()->style.border_size);
		}
		draw->ChannelsSetCurrent(1);
		draw->ChannelsMerge();

		if (context.disabled)
		{
			end_disabled();
		}

		ImGui::EndChild();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(3);
		ImGui::PopID();
	}

	bool card_button(StringView title, const CardOptions& options, const ActionRef& action)
	{
		cleanup_states();
		if (has_text(title))
		{
			imgui_push_id(title);
		}
		else
		{
			ImGui::PushID("card_button");
		}

		const ImGuiID id     = ImGui::GetID("card_button");
		AnimState& anim      = state_for(id);
		const float rounding = options.rounding >= 0.0f ? options.rounding : active_context()->style.rounding;
		const float padding  = options.padding >= 0.0f ? options.padding : active_context()->style.padding;
		const float spacing  = options.spacing >= 0.0f ? options.spacing : active_context()->style.spacing;
		const Vec4 accent    = has_color(options.accent) ? options.accent : active_context()->style.colors.accent;
		Vec4 bg     = has_color(options.background_color) ? options.background_color : active_context()->style.colors.panel;
		Vec4 border = has_color(options.border_color) ? options.border_color : active_context()->style.colors.border;

		Vec2 size = resolve(options.size);
		if (size.x <= 0.0f)
		{
			size.x = ImGui::GetContentRegionAvail().x;
		}
		if (size.y <= 0.0f)
		{
			size.y = padding * 2.0f + active_context()->style.frame_height;
		}

		const ImVec2 pos = ImGui::GetCursorScreenPos();
		if (options.disabled)
		{
			ImGui::BeginDisabled();
		}
		ImGui::InvisibleButton("##card_button", to_imvec(size));
		if (options.disabled)
		{
			ImGui::EndDisabled();
		}
		const ImVec2 item_end_pos = ImGui::GetCursorScreenPos();

		const bool hovered = options.hoverable && !options.disabled && ImGui::IsItemHovered();
		const bool active  = !options.disabled && ImGui::IsItemActive();
		const bool clicked = !options.disabled && ImGui::IsItemClicked();
		anim.hover         = approach(anim.hover, hovered ? 1.0f : 0.0f);
		anim.active        = approach(anim.active, active ? 1.0f : 0.0f * 1.5f);
		anim.selected      = approach(anim.selected, options.selected ? 1.0f : 0.0f);

		const bool hover_scaled = anim.hover > 0.0f && (active_context()->style.hover_padding.x != 0.0f ||
		                                                active_context()->style.hover_padding.y != 0.0f);
		if (hover_scaled)
		{
			push_render_scale(make_hover_render_scale(to_imvec(size), anim.hover), RenderScaleFlags::StartFromLastItemBounds);
		}
		const bool press_scaled = anim.active > 0.0f;
		if (press_scaled)
		{
			push_render_scale(make_press_render_scale(to_imvec(size), anim.active), RenderScaleFlags::StartFromLastItemBounds);
		}

		const InteractiveRect rect = make_interactive_rect(pos, to_imvec(size));

		if (options.background)
		{
			bg = Math::lerp(bg, active_context()->style.colors.background_hovered, anim.hover * 0.40f);
			bg = Math::lerp(bg, active_context()->style.colors.background_active, anim.active * 0.38f);
			bg = Math::lerp(bg, with_alpha(accent, 1.0f), anim.selected * 0.12f);
		}
		border = Math::lerp(border, accent, anim.hover * 0.30f + anim.selected * 0.55f + anim.active * 0.20f);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		if (options.elevation > 0.0f)
		{
			ShadowOptions shadow = scaled_shadow(current_shadow(), options.elevation);
			if (options.disabled)
			{
				shadow.color.w *= 0.55f;
			}
			draw_shadow_rect(draw, rect.min, rect.max, rounding * rect.rounding_scale, shadow);
		}
		if (options.background)
		{
			draw->AddRectFilled(rect.min, rect.max, col_u32(bg, options.disabled ? 0.55f : 1.0f), rounding * rect.rounding_scale);
		}
		if (anim.selected > 0.01f)
		{
			draw->AddRectFilled(rect.min, ImVec2(rect.min.x + 3.0f, rect.max.y), col_u32(accent, 0.95f * anim.selected),
			                    rounding * rect.rounding_scale, ImDrawFlags_RoundCornersLeft);
		}

		draw->PushClipRect(rect.min, rect.max, true);
		ImGui::SetCursorScreenPos(ImVec2(rect.min.x + padding, rect.min.y + padding));
		action();

		ImGui::SetCursorScreenPos(item_end_pos);
		ImGui::Dummy(ImVec2(0.0f, 0.0f));
		draw->PopClipRect();
		if (options.border)
		{
			const float alpha = options.selected ? 1.0f : Math::lerp(0.85f, 1.0f, anim.hover * 0.65f);
			draw->AddRect(rect.min, rect.max, col_u32(border, alpha * (options.disabled ? 0.7f : 1.0f)),
			              rounding * rect.rounding_scale, 0, active_context()->style.border_size);
		}

		if (press_scaled)
		{
			pop_render_scale();
		}
		if (hover_scaled)
		{
			pop_render_scale();
		}

		ImGui::PopID();
		return clicked;
	}

	/////////////////////// LAYOUT AND SCROLLING ///////////////////////

	bool begin_horizontal(StringView id_text, Size size, float align)
	{
		String id_storage(id_text);
		ImGui::BeginHorizontal(id_storage.c_str(), to_imvec(resolve(size)), align);
		return true;
	}

	bool begin_horizontal(const void* id, Size size, float align)
	{
		ImGui::BeginHorizontal(id, to_imvec(resolve(size)), align);
		return true;
	}

	bool begin_horizontal(int id, Size size, float align)
	{
		ImGui::BeginHorizontal(id, to_imvec(resolve(size)), align);
		return true;
	}

	void end_horizontal()
	{
		ImGui::EndHorizontal();
	}

	bool begin_vertical(StringView id_text, Size size, float align)
	{
		String id_storage(id_text);
		ImGui::BeginVertical(id_storage.c_str(), to_imvec(resolve(size)), align);
		return true;
	}

	bool begin_vertical(const void* id, Size size, float align)
	{
		ImGui::BeginVertical(id, to_imvec(resolve(size)), align);
		return true;
	}

	bool begin_vertical(int id, Size size, float align)
	{
		ImGui::BeginVertical(id, to_imvec(resolve(size)), align);
		return true;
	}

	void end_vertical()
	{
		ImGui::EndVertical();
	}

	void spring(float weight, float spacing)
	{
		ImGui::Spring(weight, spacing);
	}

	void suspend_layout()
	{
		ImGui::SuspendLayout();
	}

	void resume_layout()
	{
		ImGui::ResumeLayout();
	}

	void separator()
	{
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Separator, to_imvec(active_context()->style.colors.border));
		ImGui::Separator();
		ImGui::PopStyleColor();
		ImGui::Spacing();
	}

	void spacing(Unit amount)
	{
		const float resolved_amount = resolve(amount, Axis::Y);
		if (resolved_amount < 0.0f)
		{
			ImGui::Spacing();
			return;
		}
		ImGui::Dummy(ImVec2(0.0f, resolved_amount));
	}

	void new_line()
	{
		ImGui::NewLine();
	}

	void dummy(const Size& size)
	{
		ImGui::Dummy(to_imvec(resolve(size)));
	}

	void same_line(float offset_from_start_x, float spacing_value)
	{
		ImGui::SameLine(offset_from_start_x, spacing_value);
	}

	void indent(Unit indent_w)
	{
		ImGui::Indent(resolve(indent_w, Axis::X));
	}

	void unindent(Unit indent_w)
	{
		ImGui::Unindent(resolve(indent_w, Axis::X));
	}

	void align_text_to_frame_padding()
	{
		ImGui::AlignTextToFramePadding();
	}

	bool begin_disabled(bool disabled)
	{
		ImGui::BeginDisabled(disabled);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * (disabled ? 0.55f : 1.0f));
		active_context()->disabled_alpha_stack.push_back(active_context()->draw_alpha);

		if (disabled)
		{
			active_context()->draw_alpha *= 0.55f;
		}
		return true;
	}

	void end_disabled()
	{
		auto& stack = active_context()->disabled_alpha_stack;

		if (!stack.empty())
		{
			active_context()->draw_alpha = stack.back();
			stack.pop_back();
		}
		ImGui::PopStyleVar();
		ImGui::EndDisabled();
	}

	bool begin_animated_area(StringView id_label, bool visible)
	{
		imgui_push_id(id_label);
		const ImGuiID id  = ImGui::GetID("animated_area");
		AnimState& anim   = state_for(id);
		anim.open         = approach(anim.open, visible ? 1.0f : 0.0f);
		const bool render = visible || anim.open > 0.001f;
		if (!render)
		{
			ImGui::PopID();
			return false;
		}

		const float eased          = apply_ease(anim.open, Ease::InOutQuad);
		const ImVec2 start         = ImGui::GetCursorScreenPos();
		const float visible_height = Math::max(0.0f, anim.extra) * eased;
		const ImVec2 clip_min(start.x - 2.0f, start.y);
		const ImVec2 clip_max(start.x + ImGui::GetContentRegionAvail().x + 2.0f, start.y + visible_height);

		ImGui::PushClipRect(clip_min, clip_max, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * eased);

		area_context context;
		context.id                  = id;
		context.content_start       = start;
		context.previous_draw_alpha = active_context()->draw_alpha;
		active_context()->area_stack.push_back(context);
		active_context()->draw_alpha *= eased;
		return true;
	}

	void end_animated_area()
	{
		auto& stack = active_context()->area_stack;

		if (stack.empty())
		{
			return;
		}

		area_context context = stack.back();
		stack.pop_back();

		const ImVec2 end            = ImGui::GetCursorScreenPos();
		const float measured_height = Math::max(0.0f, end.y - context.content_start.y);
		AnimState& anim             = state_for(context.id);
		if (measured_height > 0.0f)
		{
			anim.extra = measured_height;
		}

		const float visible_height   = Math::max(anim.extra, measured_height) * apply_ease(anim.open, Ease::InOutQuad);
		active_context()->draw_alpha = context.previous_draw_alpha;
		ImGui::PopStyleVar();
		ImGui::PopClipRect();
		ImGui::SetCursorScreenPos(ImVec2(context.content_start.x, context.content_start.y + visible_height));
		ImGui::Dummy(ImVec2(0.0f, 0.0f));
		ImGui::PopID();
	}

	bool begin_scroll_area(StringView id, Size size, bool border, WindowFlags flags)
	{
		String id_storage(id);
		bool visible = ImGui::BeginChild(id_storage.c_str(), to_imvec(resolve(size)), border, to_imgui_window_flags(flags));

		if (!visible)
		{
			ImGui::EndChild();
		}

		return visible;
	}

	void end_scroll_area()
	{
		ImGui::EndChild();
	}

	void scroll_to_top()
	{
		ImGui::SetScrollY(0.0f);
	}

	void scroll_to_bottom()
	{
		ImGui::SetScrollHereY(1.0f);
	}

	/////////////////////// FRAME METRICS AND INPUT STATE ///////////////////////

	float delta_time()
	{
		return ImGui::GetIO().DeltaTime;
	}

	float frame_rate()
	{
		return ImGui::GetIO().Framerate;
	}

	double time_seconds()
	{
		return ImGui::GetTime();
	}

	int frame_count()
	{
		return ImGui::GetFrameCount();
	}

	Vec2 viewport_pos()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		return to_vec(window->Viewport->Pos);
	}

	Vec2 viewport_size()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		return to_vec(window->Viewport->Size);
	}

	Vec2 window_position()
	{
		return to_vec(ImGui::GetWindowPos());
	}

	Vec2 window_size()
	{
		return to_vec(ImGui::GetWindowSize());
	}

	float window_width()
	{
		return ImGui::GetWindowWidth();
	}

	float window_height()
	{
		return ImGui::GetWindowHeight();
	}

	float text_line_height()
	{
		return ImGui::GetTextLineHeight();
	}

	float text_line_height_with_spacing()
	{
		return ImGui::GetTextLineHeightWithSpacing();
	}

	float frame_height()
	{
		return ImGui::GetFrameHeight();
	}

	float frame_height_with_spacing()
	{
		return ImGui::GetFrameHeightWithSpacing();
	}

	Vec2 content_region_available()
	{
		return to_vec(ImGui::GetContentRegionAvail());
	}

	Vec2 cursor_position()
	{
		return to_vec(ImGui::GetCursorPos());
	}

	void cursor_position(const Vec2& position)
	{
		ImGui::SetCursorPos(to_imvec(position));
	}

	Vec2 cursor_screen_position()
	{
		return to_vec(ImGui::GetCursorScreenPos());
	}

	void cursor_screen_position(const Vec2& position)
	{
		ImGui::SetCursorScreenPos(to_imvec(position));
	}

	bool is_window_hovered()
	{
		return ImGui::IsWindowHovered();
	}

	bool is_window_focused()
	{
		return ImGui::IsWindowFocused();
	}

	bool is_window_appearing()
	{
		return ImGui::IsWindowAppearing();
	}

	Vec2 display_size()
	{
		return to_vec(ImGui::GetIO().DisplaySize);
	}

	Vec2 framebuffer_scale()
	{
		return to_vec(ImGui::GetIO().DisplayFramebufferScale);
	}

	bool wants_keyboard_capture()
	{
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	bool wants_mouse_capture()
	{
		return ImGui::GetIO().WantCaptureMouse;
	}

	bool wants_text_input()
	{
		return ImGui::GetIO().WantTextInput;
	}

	bool key_ctrl()
	{
		return ImGui::GetIO().KeyCtrl;
	}

	bool key_shift()
	{
		return ImGui::GetIO().KeyShift;
	}

	bool key_alt()
	{
		return ImGui::GetIO().KeyAlt;
	}

	bool key_super()
	{
		return ImGui::GetIO().KeySuper;
	}

	bool is_key_down(Key key_code)
	{
		return ImGui::IsKeyDown(to_imgui_key(key_code));
	}

	bool is_key_pressed(Key key_code, bool repeat)
	{
		return ImGui::IsKeyPressed(to_imgui_key(key_code), repeat);
	}

	bool is_key_released(Key key_code)
	{
		return ImGui::IsKeyReleased(to_imgui_key(key_code));
	}

	bool is_mouse_pos_valid()
	{
		return ImGui::IsMousePosValid();
	}

	bool is_mouse_down(MouseButton button)
	{
		return ImGui::IsMouseDown(to_imgui_mouse_button(button));
	}

	bool is_mouse_clicked(MouseButton button)
	{
		return ImGui::IsMouseClicked(to_imgui_mouse_button(button));
	}

	bool is_mouse_released(MouseButton button)
	{
		return ImGui::IsMouseReleased(to_imgui_mouse_button(button));
	}

	bool is_mouse_double_clicked(MouseButton button)
	{
		return ImGui::IsMouseDoubleClicked(to_imgui_mouse_button(button));
	}

	bool is_mouse_dragging(MouseButton button, float lock_threshold)
	{
		return ImGui::IsMouseDragging(to_imgui_mouse_button(button), lock_threshold);
	}

	Vec2 mouse_position()
	{
		return to_vec(ImGui::GetMousePos());
	}

	Vec2 mouse_delta()
	{
		return to_vec(ImGui::GetIO().MouseDelta);
	}

	float mouse_wheel()
	{
		return ImGui::GetIO().MouseWheel;
	}

	float mouse_wheel_h()
	{
		return ImGui::GetIO().MouseWheelH;
	}

	Vec2 mouse_drag_delta(MouseButton button, float lock_threshold)
	{
		return to_vec(ImGui::GetMouseDragDelta(to_imgui_mouse_button(button), lock_threshold));
	}

	void reset_mouse_drag_delta(MouseButton button)
	{
		ImGui::ResetMouseDragDelta(to_imgui_mouse_button(button));
	}

	bool is_mouse_hovering_rect(const Vec2& min, const Vec2& max, bool clip)
	{
		return ImGui::IsMouseHoveringRect(to_imvec(min), to_imvec(max), clip);
	}

	bool is_any_item_hovered()
	{
		return ImGui::IsAnyItemHovered();
	}

	bool is_any_item_active()
	{
		return ImGui::IsAnyItemActive();
	}

	bool is_any_item_focused()
	{
		return ImGui::IsAnyItemFocused();
	}

	bool is_item_hovered()
	{
		return ImGui::IsItemHovered();
	}

	bool is_item_active()
	{
		return ImGui::IsItemActive();
	}

	bool is_item_clicked()
	{
		return ImGui::IsItemClicked();
	}

	bool is_item_focused()
	{
		return ImGui::IsItemFocused();
	}

	bool is_item_edited()
	{
		return ImGui::IsItemEdited();
	}

	bool is_item_activated()
	{
		return ImGui::IsItemActivated();
	}

	bool is_item_deactivated()
	{
		return ImGui::IsItemDeactivated();
	}

	bool is_item_deactivated_after_edit()
	{
		return ImGui::IsItemDeactivatedAfterEdit();
	}

	bool is_item_toggled_open()
	{
		return ImGui::IsItemToggledOpen();
	}

	bool is_item_visible()
	{
		return ImGui::IsItemVisible();
	}

	bool is_mouse_hovering_item_rect()
	{
		return ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	Vec2 item_rect_min()
	{
		return to_vec(ImGui::GetItemRectMin());
	}

	Vec2 item_rect_max()
	{
		return to_vec(ImGui::GetItemRectMax());
	}

	Vec2 item_rect_size()
	{
		return to_vec(ImGui::GetItemRectSize());
	}

	Vec2 item_rect_center()
	{
		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();
		return Vec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);
	}

	void keyboard_focus_here(i32 offset)
	{
		ImGui::SetKeyboardFocusHere(offset);
	}

	/////////////////////// TEXT AND TOOLTIPS ///////////////////////

	void text_unformatted(StringView text)
	{
		ImGui::TextUnformatted(text.data(), text.data() + text.size());
	}

	void text(StringView fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		text_v(active_context()->style.colors.text, fmt, args);
		va_end(args);
	}

	void text_muted(StringView fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		text_v(active_context()->style.colors.text_muted, fmt, args);
		va_end(args);
	}

	void text_wrapped(StringView fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

		char buffer[2048];
		String format(fmt);
		std::vsnprintf(buffer, sizeof(buffer), format.c_str(), args);

		ImGui::PushStyleColor(ImGuiCol_Text, to_imvec(active_context()->style.colors.text));
		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + Math::max(1.0f, ImGui::GetContentRegionAvail().x));
		ImGui::TextUnformatted(buffer);
		ImGui::PopTextWrapPos();
		ImGui::PopStyleColor();

		va_end(args);
	}

	void text_colored(const Vec4& color, StringView fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		text_v(color, fmt, args);
		va_end(args);
	}

	void label(StringView label_text, StringView value)
	{
		text_muted(label_text);
		if (has_text(value))
		{
			ImGui::SameLine();
			text(value);
		}
	}

	void help_marker(StringView description)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
		{
			tooltip(description);
		}
	}

	void tooltip(StringView value)
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		imgui_text_unformatted(value);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}

	void tooltip_delayed(StringView value, float delay)
	{
		const ImGuiID id = ImGui::GetItemID();
		float& time      = active_context()->hover_time[id];
		if (ImGui::IsItemHovered())
		{
			time += dt();
		}
		else
		{
			time = 0.0f;
		}
		if (time >= delay)
		{
			tooltip(value);
		}
	}

	void tooltip_if_hovered(StringView value, float delay)
	{
		if (!ImGui::IsItemHovered())
		{
			return;
		}
		if (delay <= 0.0f)
		{
			tooltip(value);
			return;
		}
		tooltip_delayed(value, delay);
	}

	void help_tooltip(StringView description)
	{
		help_marker(description);
	}

	void clipboard_text(StringView text)
	{
		String storage(has_text(text) ? text : StringView());
		ImGui::SetClipboardText(storage.c_str());
	}

}// namespace Trinex::UI
