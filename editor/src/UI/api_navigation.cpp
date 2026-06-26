#include "api_internal.hpp"

namespace Trinex::UI
{
	/////////////////////// HEADERS, TREES AND NAVIGATION ///////////////////////

	bool begin_collapsing_header(const char* label, const HeaderOptions& options)
	{
		cleanup_states();
		ImGui::PushID(label);
		const ImGuiID id = ImGui::GetID("header_open");
		bool& stored     = active_context()->open[id];
		static std::unordered_map<ImGuiID, bool> initialized;
		if (!initialized[id])
		{
			stored          = options.open != nullptr ? *options.open : options.default_open;
			initialized[id] = true;
		}
		bool open       = options.open != nullptr ? *options.open : stored;
		AnimState& anim = state_for(id);
		if (animated_row(label, options.icon, options.right_text, open, options.disabled,
		                 ImVec2(0, active_context()->style.frame_height), options.accent, true, anim.open))
		{
			open = !open;
			if (options.open != nullptr)
			{
				*options.open = open;
			}
			else
			{
				stored = open;
			}
		}

		anim.open = approach(anim.open, open ? 1.0f : 0.0f, active_context()->style.animation_speed);
		if (open && anim.open > 0.995f)
		{
			anim.open = 1.0f;
		}
		else if (!open && anim.open < 0.005f)
		{
			anim.open = 0.0f;
		}

		ImGui::PopID();
		if (begin_animated_area(label, open))
		{
			return true;
		}
		return false;
	}

	void end_collapsing_header()
	{
		end_animated_area();
	}

	bool begin_section_header(const char* label, const HeaderOptions& options)
	{
		spacing(2.0f);
		return begin_collapsing_header(label, options);
	}

	void end_section_header()
	{
		end_collapsing_header();
	}

	bool tree_node(const char* label, const TreeNodeOptions& options)
	{
		ImGui::PushID(label);
		const ImGuiID id = ImGui::GetID("tree_open");
		bool& stored     = active_context()->open[id];
		static std::unordered_map<ImGuiID, bool> initialized;
		if (!initialized[id])
		{
			stored          = options.open != nullptr ? *options.open : options.default_open;
			initialized[id] = true;
		}
		bool open          = options.leaf ? false : (options.open != nullptr ? *options.open : stored);
		AnimState& anim    = state_for(id);
		auto& stack        = active_context()->tree_indent_stack;
		const float indent = stack.empty() ? 0.0f : stack.back();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);
		const bool clicked = animated_row(label, options.icon, options.badge, options.selected, false,
		                                  ImVec2(ImGui::GetContentRegionAvail().x, active_context()->style.frame_height),
		                                  options.accent, !options.leaf, anim.open);
		if (clicked && !options.leaf)
		{
			open = !open;
			if (options.open != nullptr)
			{
				*options.open = open;
			}
			else
			{
				stored = open;
			}
		}

		anim.open = approach(anim.open, open ? 1.0f : 0.0f, active_context()->style.animation_speed);
		if (open && anim.open > 0.995f)
		{
			anim.open = 1.0f;
		}
		else if (!open && anim.open < 0.005f)
		{
			anim.open = 0.0f;
		}
		// Symmetric easing keeps perceived expand/collapse duration matched.
		const float eased_open    = apply_ease(anim.open, Ease::InOutQuad);
		const bool render_content = !options.leaf && (open || anim.open > 0.001f);
		if (render_content)
		{
			const ImVec2 content_start = ImGui::GetCursorScreenPos();
			const float cached_height  = std::max(0.0f, anim.extra);
			const float visible_height = cached_height * eased_open;
			const ImVec2 clip_min(content_start.x - 2.0f, content_start.y);
			const ImVec2 clip_max(content_start.x + ImGui::GetContentRegionAvail().x + 2.0f,
			                      content_start.y + std::max(0.0f, visible_height));

			// Children are submitted at full size so ImGui can measure them, then clipped
			// and the cursor is rewound in tree_pop() to consume only animated height.
			ImGui::PushClipRect(clip_min, clip_max, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * eased_open);

			tree_context context;
			context.id                  = id;
			context.logical_open        = open;
			context.open_anim           = anim.open;
			context.visible_height      = visible_height;
			context.content_start       = content_start;
			context.previous_draw_alpha = active_context()->draw_alpha;
			active_context()->tree_stack.push_back(context);
			active_context()->draw_alpha *= eased_open;
			active_context()->tree_indent_stack.push_back(indent + 18.0f);
			return true;
		}
		ImGui::PopID();
		return false;
	}

	void tree_pop()
	{
		auto& tree_stack   = active_context()->tree_stack;
		auto& indent_stack = active_context()->tree_indent_stack;

		if (tree_stack.empty())
		{
			return;
		}

		tree_context context = tree_stack.back();
		tree_stack.pop_back();

		if (!indent_stack.empty())
		{
			indent_stack.pop_back();
		}

		ImVec2 content_end          = ImGui::GetCursorScreenPos();
		const float measured_height = std::max(0.0f, content_end.y - context.content_start.y);
		AnimState& anim             = state_for(context.id);
		if (context.logical_open && measured_height > 0.0f)
		{
			anim.extra = measured_height;
		}

		const float eased_open     = apply_ease(anim.open, Ease::InOutQuad);
		const float cached_height  = std::max(anim.extra, measured_height);
		const float visible_height = cached_height * eased_open;

		active_context()->draw_alpha = context.previous_draw_alpha;
		ImGui::PopStyleVar();
		ImGui::PopClipRect();

		ImGui::SetCursorScreenPos(ImVec2(context.content_start.x, context.content_start.y + visible_height));
		ImGui::Dummy(ImVec2(0.0f, 0.0f));
		ImGui::PopID();
	}

	bool tree_leaf(const char* label, const TreeNodeOptions& options)
	{
		TreeNodeOptions copy = options;
		copy.leaf            = true;
		auto& stack          = active_context()->tree_indent_stack;
		const float indent   = stack.empty() ? 0.0f : stack.back();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);
		return animated_row(label, copy.icon, copy.badge, copy.selected, false,
		                    ImVec2(ImGui::GetContentRegionAvail().x, active_context()->style.frame_height), copy.accent, false,
		                    0.0f);
	}

	bool tree_item(const char* label, const TreeNodeOptions& options)
	{
		return tree_leaf(label, options);
	}

	bool selectable_tree_item(const char* label, bool selected, const TreeNodeOptions& options)
	{
		TreeNodeOptions copy = options;
		copy.selected        = selected;
		return tree_leaf(label, copy);
	}

	bool begin_tab_bar(const char* id)
	{
		ImGui::PushID(id);
		ImGui::BeginGroup();
		return true;
	}

	void end_tab_bar()
	{
		ImGui::EndGroup();
		ImGui::PopID();
	}

	bool tab(const char* label, bool selected, const Vec2& size)
	{
		ButtonOptions options;
		options.size       = size.x > 0.0f || size.y > 0.0f ? size : Vec2(0.0f, 30.0f);
		options.ghost      = !selected;
		options.accent     = active_context()->style.colors.accent;
		const bool clicked = button(label, options);
		const ImVec2 min   = ImGui::GetItemRectMin();
		const ImVec2 max   = ImGui::GetItemRectMax();
		AnimState& anim    = state_for(ImGui::GetID(label));
		anim.selected      = approach(anim.selected, selected ? 1.0f : 0.0f, active_context()->style.animation_speed);
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x + 8.0f, max.y - 3.0f),
		                                          ImVec2(Math::lerp(min.x + 8.0f, max.x - 8.0f, anim.selected), max.y),
		                                          col_u32(active_context()->style.colors.accent, anim.selected), 2.0f);
		return clicked;
	}

	bool sidebar_item(const char* label, bool selected, const char* icon, const char* badge_text)
	{
		return animated_row(label, icon, badge_text, selected, false,
		                    ImVec2(ImGui::GetContentRegionAvail().x, active_context()->style.frame_height),
		                    active_context()->style.colors.accent, false, 0.0f);
	}

	bool nav_item(const char* label, bool selected, const char* icon)
	{
		return sidebar_item(label, selected, icon, nullptr);
	}

	bool breadcrumb(const char* label, bool current)
	{
		const bool clicked = ghost_button(label, Vec2(0.0f, 26.0f));
		if (!current)
		{
			ImGui::SameLine();
			text_muted("/");
			ImGui::SameLine();
		}
		return clicked;
	}

	/////////////////////// POPUPS, MENUS AND COMMANDS ///////////////////////

	bool begin_modal(const char* name, bool* open, WindowFlags flags)
	{
		if (consume_pending_modal(name))
		{
			ImGui::OpenPopup(name);
		}

		push_window_styles(true);
		const bool visible = ImGui::BeginPopupModal(name, open, to_imgui_window_flags(flags));

		if (!visible)
		{
			pop_window_styles();
		}
		return visible;
	}

	void open_modal(const char* name)
	{
		if (name != nullptr)
		{
			active_context()->pending_modals.emplace_back(name);
		}
	}

	void end_modal()
	{
		ImGui::EndPopup();
		pop_window_styles();
	}

	bool begin_popup(const char* id, WindowFlags flags)
	{
		if (consume_pending_popup(id))
		{
			ImGui::OpenPopup(id);
		}
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, active_context()->style.rounding);
		ImGui::PushStyleColor(ImGuiCol_PopupBg, to_imvec(active_context()->style.colors.panel));
		const bool visible = ImGui::BeginPopup(id, to_imgui_window_flags(flags));
		if (!visible)
		{
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
		}
		return visible;
	}

	void open_popup(const char* id)
	{
		if (id != nullptr)
		{
			active_context()->pending_popups.emplace_back(id);
		}
	}

	void end_popup()
	{
		ImGui::EndPopup();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}

	bool begin_context_menu(const char* id)
	{
		return ImGui::BeginPopupContextItem(id);
	}

	void end_context_menu()
	{
		ImGui::EndPopup();
	}

	bool begin_menu_bar()
	{
		push_menu_bar_colors();
		const bool open = ImGui::BeginMenuBar();
		if (open)
		{
			draw_menu_bar_background();
			++active_context()->menu_bar_style_depth;
		}
		else
		{
			pop_menu_bar_colors();
		}
		return open;
	}

	void end_menu_bar()
	{
		ImGui::EndMenuBar();
		auto& depth = active_context()->menu_bar_style_depth;

		if (depth > 0)
		{
			--depth;
			pop_menu_bar_colors();
		}
	}

	bool begin_main_menu_bar()
	{
		push_menu_bar_colors();
		const bool open = ImGui::BeginMainMenuBar();
		if (open)
		{
			draw_menu_bar_background();
			++active_context()->menu_bar_style_depth;
		}
		else
		{
			pop_menu_bar_colors();
		}
		return open;
	}

	void end_main_menu_bar()
	{
		ImGui::EndMainMenuBar();
		auto& depth = active_context()->menu_bar_style_depth;

		if (depth > 0)
		{
			--depth;
			pop_menu_bar_colors();
		}
	}

	bool begin_menu(const char* label, bool enabled)
	{
		const ImGuiID id  = ImGui::GetID(label);
		AnimState& anim   = state_for(id);
		const float alpha = std::max(0.001f, apply_ease(anim.open, Ease::InOutQuad));
		ImGui::SetNextWindowBgAlpha(active_context()->style.colors.panel.w * active_context()->style.alpha * alpha);
		push_menu_popup_colors();
		const bool open = ImGui::BeginMenu(label, enabled);
		anim.open       = approach(anim.open, open ? 1.0f : 0.0f, active_context()->style.animation_speed);
		if (open && anim.open > 0.995f)
		{
			anim.open = 1.0f;
		}
		else if (!open && anim.open < 0.005f)
		{
			anim.open = 0.0f;
		}
		if (open)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * apply_ease(anim.open, Ease::InOutQuad));
			++active_context()->menu_alpha_depth;
			++active_context()->menu_popup_style_depth;
		}
		else
		{
			pop_menu_popup_colors();
		}
		return open;
	}

	void end_menu()
	{
		if (auto& depth = active_context()->menu_alpha_depth; depth > 0)
		{
			--depth;
			ImGui::PopStyleVar();
		}

		ImGui::EndMenu();
		if (auto& depth = active_context()->menu_popup_style_depth; depth > 0)
		{
			--depth;
			pop_menu_popup_colors();
		}
	}

	bool menu_item(const char* label, const char* shortcut, bool selected, bool enabled)
	{
		push_menu_popup_colors();
		const bool clicked = ImGui::MenuItem(label, shortcut, selected, enabled);
		pop_menu_popup_colors();
		return clicked;
	}

	bool menu_item(const char* label, const char* shortcut, bool* selected, bool enabled)
	{
		push_menu_popup_colors();
		const bool clicked = ImGui::MenuItem(label, shortcut, selected, enabled);
		pop_menu_popup_colors();
		return clicked;
	}

	void register_command(Context* context, const Command& command)
	{
		trinex_assert_msg(context, "UI::register_command() requires a non-empty context");
		trinex_assert_msg(has_text(command.id), "UI::register_command() requires a non-empty command id");
		trinex_assert_msg(has_text(command.name), "UI::register_command() requires a non-empty command name");

		if (!has_text(command.id) || !has_text(command.name))
		{
			return;
		}

		for (RegisteredCommand& existing : context->command_palette.commands)
		{
			if (existing.id == command.id)
			{
				existing.name        = command.name;
				existing.description = command.description != nullptr ? command.description : "";
				existing.shortcut    = command.shortcut != nullptr ? command.shortcut : "";
				existing.icon        = command.icon != nullptr ? command.icon : "";
				existing.action      = command.action;
				return;
			}
		}

		RegisteredCommand& entry = context->command_palette.commands.emplace_back();
		entry.id                 = command.id;
		entry.name               = command.name;
		entry.description        = command.description != nullptr ? command.description : "";
		entry.shortcut           = command.shortcut != nullptr ? command.shortcut : "";
		entry.icon               = command.icon != nullptr ? command.icon : "";
		entry.action             = command.action;
	}

	void register_command(const Command& command)
	{
		register_command(active_context(), command);
	}

	void execute_command(StringView cmd)
	{
		for (RegisteredCommand& existing : active_context()->command_palette.commands)
		{
			if (existing.id == cmd)
			{
				existing.action();
				return;
			}
		}
	}

	void open_command_palette()
	{
		auto& palette = active_context()->command_palette;

		palette.open                    = true;
		palette.focus_search_next_frame = true;
		palette.search[0]               = '\0';
		palette.selected_index          = 0;
		refresh_command_palette_results();
	}

	bool command_palette()
	{
		trinex_assert(active_context() && "UI::command_palette() requires an active UI context");

		auto& style   = active_context()->style;
		auto& palette = active_context()->command_palette;

		AnimState& palette_anim      = state_for(ImGui::GetID("##command_palette_popup_anim"));
		AnimState& palette_blur_anim = state_for(ImGui::GetID("##command_palette_blur_anim"));
		palette_anim.open            = approach(palette_anim.open, palette.open ? 1.0f : 0.0f, style.animation_speed);
		palette_blur_anim.open = approach(palette_blur_anim.open, palette.open ? 1.0f : 0.0f, style.animation_speed * 0.45f);

		if (!palette.open && palette_anim.open <= 0.0f && palette_blur_anim.open <= 0.0f)
		{
			return false;
		}

		if (palette.open && palette_anim.open > 0.995f)
		{
			palette_anim.open = 1.0f;
		}
		else if (!palette.open && palette_anim.open < 0.005f)
		{
			palette_anim.open = 0.0f;
		}
		if (palette.open && palette_blur_anim.open > 0.995f)
		{
			palette_blur_anim.open = 1.0f;
		}
		else if (!palette.open && palette_blur_anim.open < 0.005f)
		{
			palette_blur_anim.open = 0.0f;
		}

		const float eased_open         = apply_ease(palette_anim.open, Ease::InOutQuad);
		const float eased_blur         = apply_ease(palette_blur_anim.open, Ease::InOutQuad);
		const float popup_visual_alpha = palette.open ? eased_open : eased_blur;

		refresh_command_palette_results();

		bool execute_requested       = false;
		int execute_registry_index   = -1;
		bool ensure_selected_visible = palette.focus_search_next_frame;
		const bool has_results       = !palette.filtered_indices.empty();
		const int page_step          = 8;
		bool keyboard_navigation     = false;

		if (ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			palette.open = false;
		}
		if (has_results && ImGui::IsKeyPressed(ImGuiKey_DownArrow))
		{
			palette.selected_index  = Math::clamp<u32>(palette.selected_index + 1, 0, palette.filtered_indices.size() - 1);
			ensure_selected_visible = true;
			keyboard_navigation     = true;
		}
		if (has_results && ImGui::IsKeyPressed(ImGuiKey_UpArrow))
		{
			palette.selected_index  = Math::clamp<u32>(palette.selected_index - 1, 0, palette.filtered_indices.size() - 1);
			ensure_selected_visible = true;
			keyboard_navigation     = true;
		}
		if (has_results && ImGui::IsKeyPressed(ImGuiKey_PageDown))
		{
			palette.selected_index = Math::clamp<u32>(palette.selected_index + page_step, 0, palette.filtered_indices.size() - 1);
			ensure_selected_visible = true;
			keyboard_navigation     = true;
		}
		if (has_results && ImGui::IsKeyPressed(ImGuiKey_PageUp))
		{
			palette.selected_index = Math::clamp<u32>(palette.selected_index - page_step, 0, palette.filtered_indices.size() - 1);
			ensure_selected_visible = true;
			keyboard_navigation     = true;
		}
		if (has_results && ImGui::IsKeyPressed(ImGuiKey_Enter))
		{
			execute_requested      = true;
			execute_registry_index = palette.filtered_indices[palette.selected_index];
			keyboard_navigation    = true;
		}

		ImGuiViewport* viewport        = ImGui::GetMainViewport();
		const float palette_max_width  = std::max(260.0f, viewport->WorkSize.x - 24.0f);
		const float palette_max_height = std::max(220.0f, viewport->WorkSize.y - 24.0f);
		const float width              = Math::clamp(viewport->WorkSize.x * 0.52f, std::min(360.0f, palette_max_width),
		                                             std::min(720.0f, palette_max_width));
		const float max_height         = Math::clamp(viewport->WorkSize.y * 0.65f, std::min(260.0f, palette_max_height),
		                                             std::min(520.0f, palette_max_height));
		const ImVec2 target_size(width, max_height);
		const ImVec2 target_pos(viewport->WorkPos.x + (viewport->WorkSize.x - target_size.x) * 0.5f,
		                        viewport->WorkPos.y + std::max(24.0f, (viewport->WorkSize.y - target_size.y) * 0.22f));
		const float popup_scale = Math::lerp(0.965f, 1.0f, eased_open);
		const ImVec2 animated_size(target_size.x * popup_scale, target_size.y * Math::lerp(0.90f, 1.0f, eased_open));
		const ImVec2 animated_pos(target_pos.x + (target_size.x - animated_size.x) * 0.5f,
		                          target_pos.y + (target_size.y - animated_size.y) * 0.5f - (1.0f - eased_open) * 18.0f);
		const float rounding = active_context()->style.rounding + 2.0f;
		const float padding  = active_context()->style.padding;
		const float spacing  = active_context()->style.spacing;

		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowPos(animated_pos);
		ImGui::SetNextWindowSize(animated_size);
		ImGui::SetNextWindowBgAlpha(0.0f);
		if (!ImGui::IsPopupOpen("##command_palette_popup"))
		{
			ImGui::OpenPopup("##command_palette_popup");
		}
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, rounding);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg,
		                      to_imvec(with_alpha(active_context()->style.colors.background, 0.28f * popup_visual_alpha)));

		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
		                               ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking |
		                               ImGuiWindowFlags_NoNavFocus;
		const bool visible = ImGui::BeginPopupModal("##command_palette_popup", nullptr, flags);
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(3);

		if (!visible)
		{
			palette.open = false;
			return false;
		}

		{
			BlurOptions options;
			options.radius = 12.0f * eased_blur;
			options.sigma  = 4.0f * eased_blur;
			options.tint   = Vec4(0.0f, 0.0f, 0.0f, 0.0f);

			blur(to_vec(viewport->Pos), to_vec(viewport->Pos + viewport->Size), DrawList::Default, options);
		}

		const float previous_draw_alpha = active_context()->draw_alpha;
		active_context()->draw_alpha *= popup_visual_alpha;
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * popup_visual_alpha);

		BlurOptions palette_blur;
		palette_blur.radius   = 12.0f * eased_blur;
		palette_blur.sigma    = 6.0f;
		palette_blur.spread   = 0.0f;
		palette_blur.rounding = rounding;
		palette_blur.tint     = Vec4(0, 0, 0, 0);
		push_blur(palette_blur);

		Shadow palette_shadow;
		palette_shadow.offset = Vec2(0.0f, 10.0f);
		palette_shadow.blur   = 28.0f;
		palette_shadow.spread = 0.0f;
		palette_shadow.color  = Vec4(0.0f, 0.0f, 0.0f, 0.22f * popup_visual_alpha);
		push_shadow(palette_shadow);

		GlassOptions palette_glass;
		palette_glass.opacity       = 0.8f;
		palette_glass.tint          = Vec4(0.10f, 0.12f, 0.16f, 0.58f * popup_visual_alpha);
		palette_glass.border_color  = Vec4(1.0f, 1.0f, 1.0f, 0.10f * popup_visual_alpha);
		palette_glass.highlight     = Vec4(1.0f, 1.0f, 1.0f, 0.08f * popup_visual_alpha);
		palette_glass.border        = true;
		palette_glass.background    = true;
		palette_glass.highlight_top = true;
		palette_glass.rounding      = rounding;
		palette_glass.padding       = padding;

		if (!begin_glass_panel("##command_palette_glass", Vec2(animated_size.x, animated_size.y), palette_glass))
		{
			pop_shadow();
			pop_blur();
			ImGui::PopStyleVar();
			active_context()->draw_alpha = previous_draw_alpha;
			ImGui::EndPopup();
			return false;
		}

		push_input_frame_styles(1.0f);
		if (palette.focus_search_next_frame)
		{
			ImGui::SetKeyboardFocusHere();
			palette.focus_search_next_frame = false;
		}
		const bool search_submitted = ImGui::InputTextWithHint("##command_palette_search", "Search commands...", palette.search,
		                                                       sizeof(palette.search), ImGuiInputTextFlags_EnterReturnsTrue);
		pop_input_frame_styles();
		if (ImGui::IsItemEdited())
		{
			palette.selected_index = 0;
			refresh_command_palette_results();
		}

		if (!execute_requested && search_submitted && has_results)
		{
			execute_requested      = true;
			execute_registry_index = palette.filtered_indices[palette.selected_index];
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		const float list_height =
		        std::max(120.0f, animated_size.y - active_context()->style.frame_height - padding * 3.0f - 10.0f);
		ImGui::BeginChild("##command_palette_results", ImVec2(0.0f, list_height), false, ImGuiWindowFlags_NoBackground);

		if (!has_results)
		{
			const ImVec2 avail = ImGui::GetContentRegionAvail();
			const ImVec2 start = ImGui::GetCursorScreenPos();
			const ImVec2 center(start.x + avail.x * 0.5f, start.y + std::max(72.0f, avail.y * 0.38f));
			const char* empty_title = "No commands found";
			const char* empty_desc  = "Try a different search query.";
			const ImVec2 title_size = ImGui::CalcTextSize(empty_title);
			const ImVec2 desc_size  = ImGui::CalcTextSize(empty_desc);
			ImDrawList* draw        = ImGui::GetWindowDrawList();
			draw->AddText(ImVec2(center.x - title_size.x * 0.5f, center.y - 12.0f), col_u32(active_context()->style.colors.text),
			              empty_title);
			draw->AddText(ImVec2(center.x - desc_size.x * 0.5f, center.y + 10.0f),
			              col_u32(active_context()->style.colors.text_muted), empty_desc);
			ImGui::Dummy(ImVec2(avail.x, std::max(140.0f, avail.y)));
		}
		else
		{
			for (int visible_index = 0; visible_index < static_cast<int>(palette.filtered_indices.size()); ++visible_index)
			{
				const RegisteredCommand& command = palette.commands[palette.filtered_indices[visible_index]];
				const bool selected              = visible_index == palette.selected_index;
				const bool enabled               = static_cast<bool>(command.action);
				const bool has_icon              = !command.icon.empty();
				const bool has_desc              = !command.description.empty();
				const bool has_shortcut          = !command.shortcut.empty();
				const float row_height           = has_desc ? 52.0f : 34.0f;
				const ImVec2 row_pos             = ImGui::GetCursorScreenPos();
				const float row_width            = ImGui::GetContentRegionAvail().x;

				ImGui::PushID(command.id.c_str());
				ImGui::InvisibleButton("##command_row", ImVec2(row_width, row_height));
				const bool hovered       = ImGui::IsItemHovered();
				const ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
				const bool mouse_selects_row =
				        hovered && !keyboard_navigation &&
				        ((mouse_delta.x != 0.0f || mouse_delta.y != 0.0f) || ImGui::IsMouseClicked(ImGuiMouseButton_Left));
				if (mouse_selects_row)
				{
					palette.selected_index = visible_index;
				}
				if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && enabled)
				{
					execute_requested      = true;
					execute_registry_index = palette.filtered_indices[visible_index];
				}
				if (selected && ensure_selected_visible)
				{
					ImGui::SetScrollHereY(0.5f);
				}

				ImDrawList* draw = ImGui::GetWindowDrawList();
				const ImVec2 min = row_pos;
				const ImVec2 max = add(row_pos, ImVec2(row_width, row_height));
				Vec4 row_bg      = active_context()->style.colors.panel;
				if (selected)
				{
					row_bg = mix_color(active_context()->style.colors.background_active, active_context()->style.colors.accent,
					                   0.18f);
				}
				else if (hovered)
				{
					row_bg = active_context()->style.colors.background_hovered;
				}
				if (selected || hovered)
				{
					draw->AddRectFilled(min, max, col_u32(row_bg), active_context()->style.rounding);
				}

				const float icon_x       = min.x + padding * 0.75f;
				const float text_x       = icon_x + (has_icon ? 28.0f : 0.0f);
				const float shortcut_pad = has_shortcut ? ImGui::CalcTextSize(command.shortcut.c_str()).x + padding : 0.0f;
				const float wrap_w       = std::max(80.0f, row_width - (text_x - min.x) - shortcut_pad - padding);
				const float text_y       = min.y + 8.0f;
				const float alpha_mul    = enabled ? 1.0f : 0.48f;

				if (has_icon)
				{
					draw->AddText(
					        ImVec2(icon_x, min.y + (row_height - ImGui::GetTextLineHeight()) * 0.5f),
					        col_u32(selected ? active_context()->style.colors.accent : active_context()->style.colors.text_muted,
					                alpha_mul),
					        command.icon.c_str());
				}
				draw->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(text_x, text_y),
				              col_u32(active_context()->style.colors.text, alpha_mul), command.name.c_str(), nullptr, wrap_w);
				if (has_desc)
				{
					draw->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(text_x, text_y + 18.0f),
					              col_u32(active_context()->style.colors.text_muted, alpha_mul), command.description.c_str(),
					              nullptr, wrap_w);
				}
				if (has_shortcut)
				{
					const ImVec2 shortcut_size = ImGui::CalcTextSize(command.shortcut.c_str());
					draw->AddText(ImVec2(max.x - shortcut_size.x - padding, min.y + 8.0f),
					              col_u32(active_context()->style.colors.text_muted, alpha_mul), command.shortcut.c_str());
				}
				ImGui::PopID();

				if (visible_index + 1 < palette.filtered_indices.size())
				{
					ImGui::Dummy(ImVec2(0.0f, spacing * 0.35f));
				}
			}
		}

		ImGui::EndChild();

		if (!palette.open && palette_anim.open <= 0.0f && palette_blur_anim.open <= 0.0f)
		{
			ImGui::CloseCurrentPopup();
		}

		end_glass_panel();
		pop_shadow();
		pop_blur();
		ImGui::PopStyleVar();
		active_context()->draw_alpha = previous_draw_alpha;
		ImGui::EndPopup();

		if (execute_requested && execute_registry_index >= 0 && execute_registry_index < palette.commands.size())
		{
			Action& action = palette.commands[execute_registry_index].action;

			if (action)
			{
				palette.open                    = false;
				palette.focus_search_next_frame = false;
				ImGui::CloseCurrentPopup();
				action();
				return true;
			}
		}
		return false;
	}

	/////////////////////// ICONS ///////////////////////

	void icon(const char* value, FontSize size)
	{
		if (value == nullptr)
		{
			return;
		}

		push_icons_font(size);
		ImGui::TextUnformatted(value);
		pop_font();
	}

	void icon_colored(const Vec4& color, const char* value, FontSize size)
	{
		if (value == nullptr)
		{
			return;
		}

		push_icons_font(size);
		ImGui::TextColored(to_imvec(color), "%s", value);
		pop_font();
	}

	/////////////////////// FEEDBACK AND DATA VIEWS ///////////////////////

	void notification(const char* message, const NotificationOptions& options)
	{
		Notification n;
		n.id           = active_context()->next_notification_id++;
		n.title        = options.title != nullptr ? options.title : "";
		n.message      = message != nullptr ? message : "";
		n.kind         = options.kind;
		n.duration     = std::max(0.1f, options.duration);
		n.action_label = options.action_label != nullptr ? options.action_label : "";
		n.action       = options.action;
		active_context()->notifications.push_back(std::move(n));
	}

	ConfirmResult confirmation(const char* title, const char* message, const char* confirm_text, const char* cancel_text,
	                           bool danger)
	{
		ConfirmResult result = ConfirmResult::Undefined;
		bool open            = true;
		if (begin_modal(title, &open))
		{
			text("%s", message != nullptr ? message : "");
			spacing();
			if ((danger ? danger_button(confirm_text) : button(confirm_text)))
			{
				result = ConfirmResult::Confirmed;
				ImGui::CloseCurrentPopup();
			}
			same_line();
			if (ghost_button(cancel_text))
			{
				result = ConfirmResult::Canceled;
				ImGui::CloseCurrentPopup();
			}
			end_modal();
		}
		if (!open && result == ConfirmResult::Undefined)
		{
			result = ConfirmResult::Canceled;
		}
		return result;
	}

	void badge(const char* value, const Vec4& color)
	{
		const Vec4 c     = has_color(color) ? color : active_context()->style.colors.accent;
		const ImVec2 ts  = ImGui::CalcTextSize(value);
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size(ts.x + 12.0f, ts.y + 6.0f);
		ImGui::Dummy(size);
		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->AddRectFilled(pos, add(pos, size), col_u32(with_alpha(c, 0.22f)), size.y * 0.5f);
		draw->AddText(ImVec2(pos.x + 6.0f, pos.y + 3.0f), col_u32(c), value);
	}

	void pill(const char* value, const Vec4& color)
	{
		badge(value, color);
	}

	void status_dot(const Vec4& color, float radius)
	{
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::Dummy(ImVec2(radius * 2.0f, radius * 2.0f));
		ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(pos.x + radius, pos.y + radius), radius,
		                                            col_u32(has_color(color) ? color : active_context()->style.colors.success));
	}

	void key_value_row(const char* key, const char* value)
	{
		text_muted("%s", key);
		ImGui::SameLine(ImGui::GetWindowWidth() * 0.45f);
		text("%s", value);
	}

	void property_row(const char* row_label, const Function<void()>& content, float label_width)
	{
		ImGui::PushID(row_label);
		ImGui::AlignTextToFramePadding();
		text_muted("%s", row_label);
		ImGui::SameLine(label_width);
		if (content)
		{
			content();
		}
		ImGui::PopID();
	}

	bool property_bool(const char* label, bool* value, bool use_checkbox, float label_width)
	{
		bool changed = false;
		property_row(label, [&] { changed = use_checkbox ? checkbox("##value", value) : toggle("##value", value); }, label_width);
		return changed;
	}

	bool property_float(const char* label, float* value, float min, float max, const char* format, float label_width)
	{
		bool changed = false;
		property_row(label, [&] { changed = slider("##value", value, min, max, format); }, label_width);
		return changed;
	}

	bool property_int(const char* label, int* value, int min, int max, const char* format, float label_width)
	{
		bool changed = false;
		property_row(label, [&] { changed = slider("##value", value, min, max, format); }, label_width);
		return changed;
	}

	bool property_text(const char* label, char* buffer, size_t buffer_size, float label_width)
	{
		bool changed = false;
		property_row(label, [&] { changed = input("##value", buffer, buffer_size); }, label_width);
		return changed;
	}

	bool property_color(const char* label, Vec4* color, bool alpha, float label_width)
	{
		bool changed = false;
		property_row(label, [&] { changed = color_edit("##value", color, alpha); }, label_width);
		return changed;
	}

	bool splitter(const char* id, float* size_a, float* size_b, float min_a, float min_b, bool vertical, float thickness)
	{
		ImGui::PushID(id);
		ImVec2 size = vertical ? ImVec2(thickness, -1.0f) : ImVec2(-1.0f, thickness);
		ImGui::InvisibleButton("##splitter", size);
		const bool active = ImGui::IsItemActive();
		if (active && size_a != nullptr && size_b != nullptr)
		{
			const float delta = vertical ? ImGui::GetIO().MouseDelta.x : ImGui::GetIO().MouseDelta.y;
			if (*size_a + delta >= min_a && *size_b - delta >= min_b)
			{
				*size_a += delta;
				*size_b -= delta;
			}
		}
		if (ImGui::IsItemHovered() || active)
		{
			ImGui::SetMouseCursor(vertical ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS);
		}
		ImGui::PopID();
		return active;
	}

	bool begin_toolbar(const char* id)
	{
		PanelOptions options;
		options.size = Vec2(0.0f, active_context()->style.frame_height + active_context()->style.padding * 2.0f);
		return begin_panel(id, options);
	}

	void end_toolbar()
	{
		end_panel();
	}

	bool begin_table(const char* id, int columns, TableFlags flags, const Vec2& outer_size, float inner_width)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(active_context()->style.padding, active_context()->style.spacing));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(active_context()->style.padding, 6.0f));
		ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, to_imvec(active_context()->style.colors.background_active));
		ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, to_imvec(active_context()->style.colors.border));
		ImGui::PushStyleColor(ImGuiCol_TableBorderLight, to_imvec(with_alpha(active_context()->style.colors.border, 0.55f)));
		ImGui::PushStyleColor(ImGuiCol_TableRowBg, to_imvec(with_alpha(active_context()->style.colors.panel, 0.30f)));
		ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt,
		                      to_imvec(with_alpha(active_context()->style.colors.background_hovered, 0.38f)));
		const bool open = ImGui::BeginTable(id, columns, to_imgui_table_flags(flags), to_imvec(outer_size), inner_width);
		if (open)
		{
			++active_context()->table_style_depth;
		}
		else
		{
			ImGui::PopStyleColor(5);
			ImGui::PopStyleVar(2);
		}
		return open;
	}

	void end_table()
	{
		ImGui::EndTable();
		if (auto& depth = active_context()->table_style_depth; depth > 0)
		{
			--depth;
			ImGui::PopStyleColor(5);
			ImGui::PopStyleVar(2);
		}
	}

	void table_column(const char* label, TableColumnFlags flags, float width_or_weight, ID user_id)
	{
		ImGui::TableSetupColumn(label, to_imgui_table_column_flags(flags), width_or_weight, to_imgui_id(user_id));
	}

	void table_headers()
	{
		ImGui::TableHeadersRow();
	}

	void table_next_row(TableRowFlags flags, float min_row_height)
	{
		ImGui::TableNextRow(to_imgui_table_row_flags(flags), min_row_height);
	}

	bool table_next_column()
	{
		return ImGui::TableNextColumn();
	}

	bool begin_list_box(const char* label, const Vec2& size)
	{
		return ImGui::BeginListBox(label, to_imvec(size));
	}

	void end_list_box()
	{
		ImGui::EndListBox();
	}

	bool list_item(const char* label, bool selected, const char* icon, const char* badge_text)
	{
		return animated_row(label, icon, badge_text, selected, false,
		                    ImVec2(ImGui::GetContentRegionAvail().x, active_context()->style.frame_height),
		                    active_context()->style.colors.accent, false, 0.0f);
	}

	bool filtered_list(const char* id, const char* filter, const char* const items[], int item_count, int* selected_index)
	{
		bool changed = false;
		ImGui::PushID(id);
		for (int i = 0; i < item_count; ++i)
		{
			if (!contains_case_insensitive(items[i], filter))
			{
				continue;
			}
			if (list_item(items[i], selected_index != nullptr && *selected_index == i))
			{
				if (selected_index != nullptr && *selected_index != i)
				{
					*selected_index = i;
					changed         = true;
				}
			}
		}
		ImGui::PopID();
		return changed;
	}

	bool begin_drag_source(DragDropFlags flags)
	{
		return ImGui::BeginDragDropSource(to_imgui_drag_drop_flags(flags));
	}

	bool drag_payload(const char* type, const void* data, size_t size, Condition condition)
	{
		return ImGui::SetDragDropPayload(type, data, size, to_imgui_cond(condition));
	}

	bool drag_payload_text(const char* type, const char* value, Condition condition)
	{
		const char* text_value = value != nullptr ? value : "";
		return ImGui::SetDragDropPayload(type, text_value, std::strlen(text_value) + 1, to_imgui_cond(condition));
	}

	void end_drag_source()
	{
		ImGui::EndDragDropSource();
	}

	bool begin_drop_target()
	{
		return ImGui::BeginDragDropTarget();
	}

	const DragDropPayload* accept_drop_payload(const char* type, DragDropFlags flags)
	{
		static DragDropPayload payload;
		if (const ImGuiPayload* accepted = ImGui::AcceptDragDropPayload(type, to_imgui_drag_drop_flags(flags)))
		{
			payload.data      = accepted->Data;
			payload.data_size = accepted->DataSize;
			payload.preview   = accepted->Preview;
			payload.delivery  = accepted->Delivery;
			return &payload;
		}
		return nullptr;
	}

	void end_drop_target()
	{
		ImGui::EndDragDropTarget();
	}

	void callout(const char* title, const char* message, NotificationKind kind)
	{
		const char* title_text   = has_text(title) ? visible_label(title) : nullptr;
		const char* message_text = has_text(message) ? message : nullptr;
		if (!has_text(title_text) && !has_text(message_text))
		{
			return;
		}

		const float rounding   = active_context()->style.rounding;
		const float padding    = active_context()->style.padding;
		const float spacing    = active_context()->style.spacing;
		const float strip_size = 4.0f;
		const Vec4 accent      = color_for_notification_kind(kind);
		const Vec4 background  = mix_color(active_context()->style.colors.panel, accent, 0.08f);
		const Vec4 border      = mix_color(active_context()->style.colors.border, accent, 0.35f);
		const char* icon       = icon_for_notification_kind(kind);

		const ImVec2 pos      = ImGui::GetCursorScreenPos();
		const float width     = std::max(1.0f, ImGui::GetContentRegionAvail().x);
		const ImVec2 icon_sz  = has_text(icon) ? ImGui::CalcTextSize(icon) : ImVec2(0.0f, 0.0f);
		const float icon_gap  = icon_sz.x > 0.0f ? spacing * 0.75f : 0.0f;
		const float content_x = pos.x + padding + strip_size + 8.0f;
		const float text_x    = content_x + icon_sz.x + icon_gap;
		const float wrap_w    = std::max(1.0f, width - (text_x - pos.x) - padding);
		const ImVec2 title_sz =
		        has_text(title_text) ? ImGui::CalcTextSize(title_text, nullptr, false, wrap_w) : ImVec2(0.0f, 0.0f);
		const ImVec2 msg_sz =
		        has_text(message_text) ? ImGui::CalcTextSize(message_text, nullptr, false, wrap_w) : ImVec2(0.0f, 0.0f);
		const float title_h = std::max(title_sz.y, icon_sz.y);
		const float gap_y   = has_text(title_text) && has_text(message_text) ? spacing * 0.35f : 0.0f;
		const float body_h  = has_text(title_text) ? title_h + gap_y + msg_sz.y : std::max(msg_sz.y, icon_sz.y);
		const ImVec2 size(width, padding * 2.0f + body_h);

		ImGui::Dummy(size);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		const ImVec2 max = add(pos, size);
		draw->AddRectFilled(pos, max, col_u32(background), rounding);
		draw->AddRect(pos, max, col_u32(border), rounding, 0, active_context()->style.border_size);
		draw->AddRectFilled(pos, ImVec2(pos.x + strip_size, max.y), col_u32(accent), rounding, ImDrawFlags_RoundCornersLeft);

		if (icon_sz.x > 0.0f)
		{
			draw->AddText(ImVec2(content_x, pos.y + padding), col_u32(accent), icon);
		}
		if (has_text(title_text))
		{
			draw->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(text_x, pos.y + padding),
			              col_u32(active_context()->style.colors.text), title_text, nullptr, wrap_w);
		}
		if (has_text(message_text))
		{
			const float message_y = pos.y + padding + (has_text(title_text) ? title_h + gap_y : 0.0f);
			draw->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(text_x, message_y),
			              col_u32(active_context()->style.colors.text_muted), message_text, nullptr, wrap_w);
		}
	}

	void banner(const char* title, const char* message, const Vec4& accent_color)
	{
		const char* title_text   = has_text(title) ? visible_label(title) : nullptr;
		const char* message_text = has_text(message) ? message : nullptr;
		if (!has_text(title_text) && !has_text(message_text))
		{
			return;
		}

		const float rounding   = active_context()->style.rounding;
		const float padding    = active_context()->style.padding * 1.35f;
		const float spacing    = active_context()->style.spacing;
		const float strip_size = 5.0f;
		const Vec4 accent      = has_color(accent_color) ? accent_color : active_context()->style.colors.accent;
		const Vec4 background  = mix_color(active_context()->style.colors.panel, accent, 0.10f);
		const Vec4 border      = mix_color(active_context()->style.colors.border, accent, 0.45f);

		const ImVec2 pos   = ImGui::GetCursorScreenPos();
		const float width  = std::max(1.0f, ImGui::GetContentRegionAvail().x);
		const float text_x = pos.x + padding + strip_size + 10.0f;
		const float wrap_w = std::max(1.0f, width - (text_x - pos.x) - padding);
		const ImVec2 title_sz =
		        has_text(title_text) ? ImGui::CalcTextSize(title_text, nullptr, false, wrap_w) : ImVec2(0.0f, 0.0f);
		const ImVec2 msg_sz =
		        has_text(message_text) ? ImGui::CalcTextSize(message_text, nullptr, false, wrap_w) : ImVec2(0.0f, 0.0f);
		const float gap_y = has_text(title_text) && has_text(message_text) ? spacing * 0.45f : 0.0f;
		const ImVec2 size(width, padding * 2.0f + title_sz.y + gap_y + msg_sz.y);

		ImGui::Dummy(size);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		const ImVec2 max = add(pos, size);
		draw->AddRectFilled(pos, max, col_u32(background), rounding);
		draw->AddRect(pos, max, col_u32(border), rounding, 0, active_context()->style.border_size);
		draw->AddRectFilled(pos, ImVec2(pos.x + strip_size, max.y), col_u32(accent), rounding, ImDrawFlags_RoundCornersLeft);

		if (has_text(title_text))
		{
			draw->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(text_x, pos.y + padding),
			              col_u32(active_context()->style.colors.text), title_text, nullptr, wrap_w);
		}
		if (has_text(message_text))
		{
			draw->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(text_x, pos.y + padding + title_sz.y + gap_y),
			              col_u32(active_context()->style.colors.text_muted), message_text, nullptr, wrap_w);
		}
	}

	bool hero(const HeroOptions& options)
	{
		const char* icon_text        = has_text(options.icon) ? options.icon : nullptr;
		const char* title_text       = has_text(options.title) ? visible_label(options.title) : nullptr;
		const char* description_text = has_text(options.description) ? options.description : nullptr;
		const char* action_text      = has_text(options.action_label) ? visible_label(options.action_label) : nullptr;
		if (!has_text(icon_text) && !has_text(title_text) && !has_text(description_text) && !has_text(action_text))
		{
			return false;
		}

		const float rounding      = options.rounding >= 0.0f ? options.rounding : active_context()->style.rounding;
		const float padding       = options.padding >= 0.0f ? options.padding : active_context()->style.padding * 2.0f;
		const float spacing       = options.spacing >= 0.0f ? options.spacing : active_context()->style.spacing;
		const Vec4 accent         = has_color(options.accent) ? options.accent : active_context()->style.colors.accent;
		const Vec4 background     = mix_color(active_context()->style.colors.panel, accent, 0.04f);
		const Vec4 border         = mix_color(active_context()->style.colors.border, accent, 0.25f);
		const float alpha_mul     = options.disabled ? 0.55f : 1.0f;
		const ImVec2 pos          = ImGui::GetCursorScreenPos();
		const float width         = options.size.x > 0.0f ? options.size.x : std::max(1.0f, ImGui::GetContentRegionAvail().x);
		const float content_width = std::max(1.0f, width - padding * 2.0f);
		const float desc_wrap     = std::max(140.0f, std::min(content_width, options.centered ? 520.0f : content_width));

		const ImVec2 icon_sz = has_text(icon_text) ? ImGui::CalcTextSize(icon_text) : ImVec2(0.0f, 0.0f);
		const ImVec2 title_sz =
		        has_text(title_text) ? ImGui::CalcTextSize(title_text, nullptr, false, content_width) : ImVec2(0.0f, 0.0f);
		const ImVec2 desc_sz = has_text(description_text) ? ImGui::CalcTextSize(description_text, nullptr, false, desc_wrap)
		                                                  : ImVec2(0.0f, 0.0f);

		float button_h = 0.0f;
		float button_w = 0.0f;
		char action_id[512];
		action_id[0] = '\0';
		if (has_text(action_text))
		{
			const ImVec2 action_sz = ImGui::CalcTextSize(action_text, nullptr, true);
			button_w               = std::max(96.0f, action_sz.x + active_context()->style.padding * 2.0f);
			button_h               = active_context()->style.frame_height;
			std::snprintf(action_id, sizeof(action_id), "%s##hero_action", action_text);
		}

		float content_h = 0.0f;
		auto add_block  = [&](bool present, float height) {
            if (!present)
            {
                return;
            }
            if (content_h > 0.0f)
            {
                content_h += spacing;
            }
            content_h += height;
		};
		add_block(has_text(icon_text), icon_sz.y);
		add_block(has_text(title_text), title_sz.y);
		add_block(has_text(description_text), desc_sz.y);
		add_block(has_text(action_text), button_h);

		const float auto_height = std::max(content_h + padding * 2.0f, 120.0f);
		const float height      = options.size.y > 0.0f ? options.size.y : auto_height;
		const ImVec2 size(width, height);

		ImGui::Dummy(size);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		const ImVec2 max = add(pos, size);
		if (options.elevation > 0.0f)
		{
			Shadow shadow = scaled_shadow(current_shadow(), options.elevation);
			if (options.disabled)
			{
				shadow.color.w *= 0.55f;
			}
			draw_shadow_rect(draw, pos, max, rounding, shadow);
		}
		if (options.background)
		{
			draw->AddRectFilled(pos, max, col_u32(background, alpha_mul), rounding);
		}
		if (options.border)
		{
			draw->AddRect(pos, max, col_u32(border, alpha_mul), rounding, 0, active_context()->style.border_size);
		}

		const float content_top =
		        options.size.y > 0.0f ? pos.y + std::max(padding, (height - content_h) * 0.5f) : pos.y + padding;
		float y              = content_top;
		const float left_x   = pos.x + padding;
		const float center_x = pos.x + width * 0.5f;

		auto block_x = [&](float block_width) { return options.centered ? center_x - block_width * 0.5f : left_x; };

		if (has_text(icon_text))
		{
			draw->AddText(ImVec2(block_x(icon_sz.x), y), col_u32(accent, alpha_mul), icon_text);
			y += icon_sz.y + spacing;
		}
		if (has_text(title_text))
		{
			draw->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(block_x(title_sz.x), y),
			              col_u32(active_context()->style.colors.text, alpha_mul), title_text, nullptr, content_width);
			y += title_sz.y + spacing;
		}
		if (has_text(description_text))
		{
			draw->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(block_x(desc_sz.x), y),
			              col_u32(active_context()->style.colors.text_muted, alpha_mul), description_text, nullptr, desc_wrap);
			y += desc_sz.y + spacing;
		}

		bool clicked = false;
		if (has_text(action_text))
		{
			ButtonOptions ButtonOptions;
			ButtonOptions.size     = Vec2(button_w, button_h);
			ButtonOptions.accent   = accent;
			ButtonOptions.disabled = options.disabled;

			const ImVec2 button_pos(block_x(button_w), y);
			ImGui::SetCursorScreenPos(button_pos);
			if (has_text(title_text))
			{
				ImGui::PushID(title_text);
			}
			else if (has_text(description_text))
			{
				ImGui::PushID(description_text);
			}
			else
			{
				ImGui::PushID("hero");
			}
			clicked = button(action_id, ButtonOptions);
			ImGui::PopID();
		}

		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y));
		ImGui::Dummy(ImVec2(0.0f, 0.0f));
		return clicked;
	}

	void empty_state(const StateOptions& options)
	{
		const char* icon        = options.icon != nullptr ? options.icon : "-";
		const char* title       = options.title != nullptr ? options.title : "Nothing here";
		const char* description = options.description != nullptr ? options.description : "";
		ImVec2 avail            = ImGui::GetContentRegionAvail();
		ImVec2 start            = ImGui::GetCursorScreenPos();
		ImVec2 center(start.x + avail.x * 0.5f, start.y + std::max(120.0f, avail.y * 0.35f));
		ImDrawList* draw        = ImGui::GetWindowDrawList();
		const ImVec2 icon_size  = ImGui::CalcTextSize(icon);
		const ImVec2 title_size = ImGui::CalcTextSize(title);
		const ImVec2 desc_size  = ImGui::CalcTextSize(description);
		draw->AddText(ImVec2(center.x - icon_size.x * 0.5f, center.y - 34.0f), col_u32(active_context()->style.colors.text_muted),
		              icon);
		draw->AddText(ImVec2(center.x - title_size.x * 0.5f, center.y - 6.0f), col_u32(active_context()->style.colors.text),
		              title);
		draw->AddText(ImVec2(center.x - desc_size.x * 0.5f, center.y + 18.0f), col_u32(active_context()->style.colors.text_muted),
		              description);
		ImGui::Dummy(ImVec2(avail.x, 140.0f));
	}

	void loading_state(const char* loading_text)
	{
		ImGui::BeginGroup();
		spinner("loading_state_spinner", 10.0f, 2.5f);
		ImGui::SameLine();
		text_muted("%s", loading_text != nullptr ? loading_text : "Loading...");
		ImGui::EndGroup();
	}

	void error_state(const char* message, const char* title)
	{
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size(ImGui::GetContentRegionAvail().x, 64.0f);
		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->AddRectFilled(pos, add(pos, size), col_u32(with_alpha(active_context()->style.colors.error, 0.14f)),
		                    active_context()->style.rounding);
		draw->AddRect(pos, add(pos, size), col_u32(active_context()->style.colors.error, 0.7f), active_context()->style.rounding);
		draw->AddText(ImVec2(pos.x + 14.0f, pos.y + 10.0f), col_u32(active_context()->style.colors.error),
		              title != nullptr ? title : "Error");
		draw->AddText(ImVec2(pos.x + 14.0f, pos.y + 34.0f), col_u32(active_context()->style.colors.text),
		              message != nullptr ? message : "");
		ImGui::Dummy(size);
	}
}// namespace Trinex::UI
