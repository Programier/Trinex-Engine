#pragma once
#include <Core/editor_config.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/stack.hpp>
#include <Core/etl/vector.hpp>
#include <Core/math/math.hpp>
#include <Core/types/path.hpp>
#include <UI/api.hpp>
#include <cstdarg>
#include <imgui.h>
#include <imgui_internal.h>

namespace Trinex::UI
{
	struct AnimState {
		f32 hover        = 0.0f;
		f32 active       = 0.0f;
		f32 focus        = 0.0f;
		f32 open         = 0.0f;
		f32 selected     = 0.0f;
		f32 value        = 0.0f;
		f32 extra        = 0.0f;
		usize last_frame = 0;

		void reset()
		{
			hover    = 0.0f;
			active   = 0.0f;
			focus    = 0.0f;
			open     = 0.0f;
			selected = 0.0f;
			value    = 0.0f;
			extra    = 0.0f;
		}
	};

	struct Notification {
		ImGuiID id = 0;
		String title;
		String message;
		NotificationKind kind = NotificationKind::Info;
		f32 duration          = 3.0f;
		f32 age               = 0.0f;
		String action_label;
		Action action;
	};

	struct TreeContext {
		ImGuiID id                = 0;
		bool logical_open         = false;
		ImVec2 content_start      = ImVec2(0.0f, 0.0f);
		float previous_draw_alpha = 1.0f;
	};

	struct AreaContext {
		ImGuiID id                = 0;
		ImVec2 content_start      = ImVec2(0.0f, 0.0f);
		float previous_draw_alpha = 1.0f;
	};

	struct PanelContext {
		bool border           = true;
		bool background       = true;
		bool draw_shadow      = false;
		float rounding        = 0.0f;
		Vec4 background_color = Vec4(0, 0, 0, 0);
		Vec4 border_color     = Vec4(0, 0, 0, 0);
		Shadow shadow;
	};

	struct GlassPanelContext {
		ImGuiID id         = 0;
		bool border        = true;
		bool background    = true;
		bool draw_shadow   = true;
		bool highlight_top = true;
		float rounding     = 0.0f;
		Vec4 tint          = Vec4(0, 0, 0, 0);
		Vec4 border_color  = Vec4(0, 0, 0, 0);
		Vec4 highlight     = Vec4(0, 0, 0, 0);
		BlurOptions blur;
		Shadow shadow;
	};

	struct CardContext {
		ImGuiID id            = 0;
		bool disabled         = false;
		bool hoverable        = true;
		bool selected         = false;
		bool border           = true;
		bool background       = true;
		float rounding        = 0.0f;
		float elevation       = 0.0f;
		Vec4 accent           = Vec4(0, 0, 0, 0);
		Vec4 background_color = Vec4(0, 0, 0, 0);
		Vec4 border_color     = Vec4(0, 0, 0, 0);
		Shadow shadow;
	};

	struct PersistentWindow {
		PersistentWindow* next = nullptr;
		Widget* widget         = nullptr;
	};

	struct RegisteredCommand {
		String id;
		String name;
		String description;
		String shortcut;
		String icon;
		Action action;
		bool is_console_command;
	};

	struct CommandPaletteState {
		bool open                    = false;
		bool focus_search_next_frame = false;
		char search[256]             = {};
		u32 selected_index           = 0;
		Vector<RegisteredCommand> commands;
		Vector<int> filtered_indices;

		inline void reset()
		{
			commands.clear();
			filtered_indices.clear();
			open                    = false;
			focus_search_next_frame = false;
			search[0]               = '\0';
			selected_index          = 0;
		}
	};

	struct RenderScale {
		struct Scope {
			Scope* next;
			ImDrawList* draw_list;
			u32 start_vertex;
			u32 start_command;

			inline Scope(ImDrawList* list, Scope* next = nullptr)
			    : next(next), draw_list(list), start_vertex(draw_list->VtxBuffer.Size), start_command(draw_list->CmdBuffer.Size)
			{}

			inline Scope(ImDrawList* list, u32 start_vertex, u32 start_command, Scope* next = nullptr)
			    : next(next), draw_list(list), start_vertex(start_vertex), start_command(start_command)
			{}
		};

		RenderScale* prev;
		Scope* scope;
		Vec2 scale;
		Vec2 pivot;
		Vec2 min;
		Vec2 max;
	};

	struct Context {
		static constexpr usize font_family_count = 3;
		static constexpr usize font_size_count   = 3;

		Trinex::Window* window = nullptr;
		ImGuiContext* context  = nullptr;
		Stack<4096> stack;
		u128 stack_memory_location                        = 0;
		ImFont* fonts[font_family_count][font_size_count] = {};
		Style style;
		Vector<Style> style_stack;
		Map<ImGuiID, AnimState> anim;
		Map<ImGuiID, bool> open;
		Map<ImGuiID, float> hover_time;
		Map<ImGuiID, float> notification_y;
		ImGuiID active_combo          = 0;
		ImVec2 active_combo_field_min = ImVec2(0.0f, 0.0f);
		ImVec2 active_combo_field_max = ImVec2(0.0f, 0.0f);
		ImVec2 active_combo_popup_min = ImVec2(0.0f, 0.0f);
		ImVec2 active_combo_popup_max = ImVec2(0.0f, 0.0f);
		int menu_bar_style_depth      = 0;
		int menu_popup_style_depth    = 0;
		int menu_alpha_depth          = 0;
		int table_style_depth         = 0;
		Vector<TreeContext> tree_stack;
		Vector<AreaContext> area_stack;
		Vector<PanelContext> panel_stack;
		Vector<GlassPanelContext> glass_panel_stack;
		Vector<CardContext> card_stack;
		Vector<Shadow> shadow_stack;
		Vector<BlurOptions> blur_stack;
		Vector<float> disabled_alpha_stack;
		Vector<String> pending_modals;
		Vector<String> pending_popups;
		Vector<Notification> notifications;
		RenderScale* render_scale     = nullptr;
		PersistentWindow* window_list = nullptr;
		CommandPaletteState command_palette;
		ImGuiID next_notification_id = 1;
		ImGuiID keybind_capture      = 0;
		float draw_alpha             = 1.0f;
		int last_cleanup_frame       = 0;
	};

	extern Context* g_context;
	namespace ui = Trinex::UI;

	using tree_context = TreeContext;
	using area_context = AreaContext;

	struct InteractiveRect {
		ImVec2 center        = ImVec2(0.0f, 0.0f);
		ImVec2 visual_size   = ImVec2(0.0f, 0.0f);
		ImVec2 min           = ImVec2(0.0f, 0.0f);
		ImVec2 max           = ImVec2(0.0f, 0.0f);
		float rounding_scale = 1.0f;
	};

	inline Context* active_context()
	{
		trinex_assert(g_context);
		return g_context;
	}

	inline constexpr usize font_size_index(ui::FontSize size)
	{
		return static_cast<usize>(size);
	}

	inline constexpr usize font_family_index(ui::FontFamily family)
	{
		return static_cast<usize>(family);
	}

	inline constexpr f32 font_size_value(ui::FontSize size)
	{
		switch (size)
		{
			case ui::FontSize::Small: return Settings::Editor::small_font_size;
			case ui::FontSize::Large: return Settings::Editor::large_font_size;
			case ui::FontSize::Normal:
			default: return Settings::Editor::normal_font_size;
		}
	}

	ui::FontSize closest_font_size(f32 pixel_size);
	Buffer load_font_data(const Path& path);
	ImFont* register_font(const Buffer& buffer, const ImFontConfig& config, const ImWchar* range, f32 size);
	void initialize_fonts(Context* ctx);
	ImFont* resolve_font(Context* ctx, ui::FontFamily family, ui::FontSize size);
	float dt();
	void* memory_copy(void* memory, usize size);
	void add_paint_callback(ImDrawList* list, ImGuiViewport* vp, Vec2 pos, Vec2 size, PaintFunction function, void* userdata,
	                        usize userdata_size);

	inline ImDrawList* resolve_draw_list(DrawList draw_list, ImGuiWindow* window)
	{
		ImGuiViewport* viewport = window ? window->Viewport : ImGui::GetMainViewport();

		switch (draw_list)
		{
			case DrawList::Background: return ImGui::GetBackgroundDrawList(viewport);
			case DrawList::Foreground: return ImGui::GetForegroundDrawList(viewport);
			case DrawList::Default:
			default: return window ? window->DrawList : ImGui::GetForegroundDrawList(viewport);
		}
	}

	inline ImDrawList* resolve_draw_list(DrawList draw_list, ImGuiWindow* window, ImGuiViewport*& viewport)
	{
		viewport = window ? window->Viewport : ImGui::GetMainViewport();

		switch (draw_list)
		{
			case DrawList::Background: return ImGui::GetBackgroundDrawList(viewport);
			case DrawList::Foreground: return ImGui::GetForegroundDrawList(viewport);
			case DrawList::Default:
			default: return window ? window->DrawList : ImGui::GetForegroundDrawList(viewport);
		}
	}

	inline float approach(float current, float target, float speed, float epsilon = 0.001f)
	{
		const float delta_time = Math::max(0.0f, dt());
		const float s          = Math::max(0.0f, speed);

		const float k    = -Math::exp(-s * delta_time) + 1.f;
		const float next = current + (target - current) * k;

		if (Math::abs(target - next) <= epsilon)
		{
			return target;
		}

		return next;
	}

	inline float approach(float current, float target)
	{
		return approach(current, target, active_context()->style.animation_speed);
	}

	AnimState& state_for(ImGuiID id);
	void cleanup_states();
	ImU32 col_u32(const ImVec4& color, float alpha_mul = 1.0f);
	PersistentWindow* find_window(Context* context, Widget* widget);

	inline ImVec2 to_imvec(const Vec2& v)
	{
		return ImVec2(v.x, v.y);
	}

	inline Vec2 to_vec(const ImVec2& v)
	{
		return Vec2(v.x, v.y);
	}

	inline ImVec4 to_imvec(const Vec4& v)
	{
		return ImVec4(v.x, v.y, v.z, v.w);
	}

	inline Vec4 to_vec(const ImVec4& v)
	{
		return Vec4(v.x, v.y, v.z, v.w);
	}

	inline ImGuiID to_imgui_id(ID value)
	{
		return static_cast<ImGuiID>(value.value());
	}

	inline ID to_ui_id(ImGuiID value)
	{
		return ID(static_cast<ID::ValueType>(value));
	}

	inline ImGuiWindowFlags to_imgui_window_flags(WindowFlags value)
	{
		return static_cast<ImGuiWindowFlags>(value);
	}

	ImGuiDockNodeFlags to_imgui_dock_window_flags(DockWindowFlags value);
	ImGuiTabItemFlags to_imgui_dock_tab_flags(DockTabFlags value);

	inline ImGuiInputTextFlags to_imgui_input_text_flags(InputTextFlags value)
	{
		return static_cast<ImGuiInputTextFlags>(value);
	}

	inline ImGuiComboFlags to_imgui_combo_flags(ComboFlags value)
	{
		return static_cast<ImGuiComboFlags>(value);
	}

	inline ImGuiSelectableFlags to_imgui_selectable_flags(SelectableFlags value)
	{
		return static_cast<ImGuiSelectableFlags>(value);
	}

	inline ImGuiColorEditFlags to_imgui_color_edit_flags(ColorEditFlags value)
	{
		return static_cast<ImGuiColorEditFlags>(value);
	}

	inline ImGuiTableFlags to_imgui_table_flags(TableFlags value)
	{
		return static_cast<ImGuiTableFlags>(value);
	}

	inline ImGuiTableColumnFlags to_imgui_table_column_flags(TableColumnFlags value)
	{
		return static_cast<ImGuiTableColumnFlags>(value);
	}

	inline ImGuiTableRowFlags to_imgui_table_row_flags(TableRowFlags value)
	{
		return static_cast<ImGuiTableRowFlags>(value);
	}

	inline ImGuiDragDropFlags to_imgui_drag_drop_flags(DragDropFlags value)
	{
		return static_cast<ImGuiDragDropFlags>(value);
	}

	inline ImGuiCond to_imgui_cond(Condition value)
	{
		return static_cast<ImGuiCond>(value);
	}

	inline ImGuiDockNodeFlags to_imgui_dock_node_flags(DockNodeFlags value)
	{
		return static_cast<ImGuiDockNodeFlags>(value);
	}

	inline ImGuiDir to_imgui_dock_dir(DockSplitDir value)
	{
		switch (value.value)
		{
			case DockSplitDir::Left: return ImGuiDir_Left;
			case DockSplitDir::Right: return ImGuiDir_Right;
			case DockSplitDir::Up: return ImGuiDir_Up;
			case DockSplitDir::Down:
			default: return ImGuiDir_Down;
		}
	}

	inline ImGuiKey to_imgui_key(Key value)
	{
		return static_cast<ImGuiKey>(value.value);
	}

	inline int to_imgui_mouse_button(MouseButton value)
	{
		return static_cast<int>(value.value);
	}

	ImU32 col_u32(const Vec4& color, float alpha_mul = 1.0f);

	inline bool has_color(const Vec4& c)
	{
		return c.x != 0.0f || c.y != 0.0f || c.z != 0.0f || c.w != 0.0f;
	}

	inline bool has_text(const char* text)
	{
		return text != nullptr && text[0] != '\0';
	}

	bool has_any_bound(const Size& min, const Size& max);
	bool has_window_class_overrides(DockWindowFlags dock_flags, DockTabFlags tab_flags);
	DockPlacement resolve_dock_placement(const WindowOptions& options);
	void apply_window_options_pre_begin(const WindowOptions& options);
	void apply_window_options_post_begin(const WindowOptions& options);
	Vec4 mix_color(const Vec4& a, const Vec4& b, float t);
	Vec4 color_for_notification_kind(NotificationKind kind);
	const char* icon_for_notification_kind(NotificationKind kind);

	inline bool blur_visible(const BlurOptions& options)
	{
		return options.radius > 0.0f || options.tint.w > 0.0f;
	}

	bool equals_case_insensitive(const char* a, const char* b);
	bool starts_with_case_insensitive(const char* text, const char* prefix);
	bool contains_case_insensitive_text(const char* text, const char* query);
	int command_match_score(const RegisteredCommand& command, const char* query);
	void refresh_command_palette_results();

	inline const Shadow& current_shadow()
	{
		auto& stack = active_context()->shadow_stack;
		return stack.empty() ? active_context()->style.shadow : stack.back();
	}

	inline const BlurOptions& current_blur()
	{
		auto& stack = active_context()->blur_stack;
		return stack.empty() ? active_context()->style.blur : stack.back();
	}

	inline bool has_shadow_override()
	{
		return !active_context()->shadow_stack.empty();
	}

	inline bool shadow_visible(const Shadow& shadow)
	{
		return shadow.color.w > 0.0f;
	}

	Shadow scaled_shadow(const Shadow& shadow, float elevation);

	inline Vec4 with_alpha(Vec4 c, float alpha)
	{
		c.w *= alpha;
		return c;
	}

	inline Vec4 default_glass_border()
	{
		return Vec4(1.0f, 1.0f, 1.0f, 0.10f);
	}

	inline ImVec2 add(ImVec2 a, ImVec2 b)
	{
		return ImVec2(a.x + b.x, a.y + b.y);
	}

	inline ImVec2 sub(ImVec2 a, ImVec2 b)
	{
		return ImVec2(a.x - b.x, a.y - b.y);
	}

	inline ImVec2 mul(ImVec2 a, float v)
	{
		return ImVec2(a.x * v, a.y * v);
	}

	Vec2 make_hover_render_scale(const ImVec2& base_size, float hover_t);
	Vec2 make_press_render_scale(const ImVec2& base_size, float active_t);
	InteractiveRect make_interactive_rect(const ImVec2& pos, const ImVec2& base_size);
	void draw_shadow_rect(ImDrawList* draw, const ImVec2& min, const ImVec2& max, float rounding, const Shadow& shadow);
	bool contains_case_insensitive(const char* text, const char* filter);

	inline bool is_modifier_key(ui::Key key)
	{
		return key == ui::Key::LeftCtrl || key == ui::Key::RightCtrl || key == ui::Key::LeftShift || key == ui::Key::RightShift ||
		       key == ui::Key::LeftAlt || key == ui::Key::RightAlt || key == ui::Key::LeftSuper || key == ui::Key::RightSuper;
	}

	inline bool keybind_mods_match(const ui::Keybind& binding)
	{
		ImGuiIO& io = ImGui::GetIO();
		return (!binding.ctrl || io.KeyCtrl) && (!binding.shift || io.KeyShift) && (!binding.alt || io.KeyAlt) &&
		       (!binding.super || io.KeySuper);
	}

	inline bool point_in_rect(ImVec2 point, ImVec2 min, ImVec2 max)
	{
		return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y;
	}

	bool consume_pending_modal(const char* name);
	bool consume_pending_popup(const char* name);
	const char* visible_label(const char* label);
	void text_v(const Vec4& color, const char* fmt, va_list args);
	void push_input_frame_styles(float focus);

	inline void pop_input_frame_styles()
	{
		ImGui::PopStyleColor();
	}

	ImVec2 default_item_size(const char* label, ImVec2 requested, float min_width = 0.0f);
	void draw_chevron(ImDrawList* draw, ImVec2 center, float size, float t, ImU32 color);
	bool animated_row(const char* label, const char* icon, const char* right_text, bool selected, bool disabled, ImVec2 size,
	                  Vec4 accent, bool draw_arrow, float arrow_t);
	Vec4 notification_color(ui::NotificationKind kind);
	void push_menu_bar_colors();
	void pop_menu_bar_colors();
	void push_menu_popup_colors();
	void pop_menu_popup_colors();
	void draw_menu_bar_background();
	void push_window_styles(bool modal);

	inline void pop_window_styles()
	{
		ImGui::PopStyleColor(5);
	}

	inline void dummy_no_spacing(const ImVec2& size = ImVec2(0.0f, 0.0f))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		ImGui::Dummy(size);
		ImGui::PopStyleVar();
	}
}// namespace Trinex::UI
