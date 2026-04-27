#include <Core/default_resources.hpp>
#include <Core/editor_config.hpp>
#include <Core/etl/vector.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/math/math.hpp>
#include <Core/reflection/class.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/texture.hpp>
#include <IconsLucide.h>
#include <RHI/context.hpp>
#include <RHI/handles.hpp>
#include <RHI/rhi.hpp>
#include <RHI/static_sampler.hpp>
#include <UI/api.hpp>
#include <UI/backend.hpp>
#include <Window/window.hpp>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <imgui.h>
#include <unordered_map>
#include <utility>

#include <Graphics/render_viewport.hpp>

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
		Function<void()> action;
	};

	struct TreeContext {
		ImGuiID id                = 0;
		bool logical_open         = false;
		float open_anim           = 0.0f;
		float visible_height      = 0.0f;
		ImVec2 content_start      = ImVec2(0.0f, 0.0f);
		float previous_draw_alpha = 1.0f;
	};

	struct AreaContext {
		ImGuiID id                = 0;
		ImVec2 content_start      = ImVec2(0.0f, 0.0f);
		float previous_draw_alpha = 1.0f;
	};

	struct PersistentWindow {
		String name;
		WindowFlags flags = WindowFlags::Undefined;
		bool open         = true;
		Function<void()> content;
	};

	struct ActiveWindowScope {
		bool* external_open          = nullptr;
		PersistentWindow* persistent = nullptr;
	};

	struct Context {
		Trinex::Window* window = nullptr;
		ImGuiContext* context  = nullptr;
		Style style;
		Vector<Style> style_stack;
		std::unordered_map<ImGuiID, AnimState> anim;
		std::unordered_map<ImGuiID, bool> open;
		std::unordered_map<ImGuiID, float> hover_time;
		std::unordered_map<ImGuiID, float> notification_y;
		ImGuiID active_combo          = 0;
		ImVec2 active_combo_field_min = ImVec2(0.0f, 0.0f);
		ImVec2 active_combo_field_max = ImVec2(0.0f, 0.0f);
		ImVec2 active_combo_popup_min = ImVec2(0.0f, 0.0f);
		ImVec2 active_combo_popup_max = ImVec2(0.0f, 0.0f);
		int menu_bar_style_depth      = 0;
		int menu_popup_style_depth    = 0;
		int menu_alpha_depth          = 0;
		int table_style_depth         = 0;
		Vector<float> tree_indent_stack;
		Vector<TreeContext> tree_stack;
		Vector<AreaContext> area_stack;
		Vector<float> disabled_alpha_stack;
		Vector<String> pending_modals;
		Vector<String> pending_popups;
		Vector<Notification> notifications;
		Vector<PersistentWindow> windows;
		Vector<ActiveWindowScope> active_window_stack;
		PersistentWindow* rendering_window = nullptr;
		ImGuiID next_notification_id       = 1;
		ImGuiID keybind_capture            = 0;
		float draw_alpha                   = 1.0f;
		int last_cleanup_frame             = 0;
	};

	Context* g_context = nullptr;
	namespace ui       = Trinex::UI;

	namespace
	{
		using tree_context      = TreeContext;
		using area_context      = AreaContext;
		using persistent_window = PersistentWindow;

		static inline Context* active_context()
		{
			trinex_assert(g_context);
			return g_context;
		}

#define g_anim active_context()->anim
#define g_open active_context()->open
#define g_hover_time active_context()->hover_time
#define g_notification_y active_context()->notification_y
#define g_active_combo active_context()->active_combo
#define g_active_combo_field_min active_context()->active_combo_field_min
#define g_active_combo_field_max active_context()->active_combo_field_max
#define g_active_combo_popup_min active_context()->active_combo_popup_min
#define g_active_combo_popup_max active_context()->active_combo_popup_max
#define g_menu_bar_style_depth active_context()->menu_bar_style_depth
#define g_menu_popup_style_depth active_context()->menu_popup_style_depth
#define g_menu_alpha_depth active_context()->menu_alpha_depth
#define g_table_style_depth active_context()->table_style_depth
#define g_tree_indent_stack active_context()->tree_indent_stack
#define g_tree_stack active_context()->tree_stack
#define g_area_stack active_context()->area_stack
#define g_disabled_alpha_stack active_context()->disabled_alpha_stack
#define g_pending_modals active_context()->pending_modals
#define g_pending_popups active_context()->pending_popups
#define g_notifications active_context()->notifications

		float dt()
		{
			return std::max(0.0f, ImGui::GetIO().DeltaTime);
		}

		float approach(float current, float target, float speed)
		{
			const float k = 1.0f - std::exp(-std::max(0.0f, speed) * dt());
			return current + (target - current) * k;
		}

		AnimState& state_for(ImGuiID id)
		{
			AnimState& state = g_anim[id];

			usize last       = state.last_frame;
			state.last_frame = ImGui::GetFrameCount();

			if (last + 1 < state.last_frame)
				state.reset();

			return state;
		}

		void cleanup_states()
		{
			const int frame = ImGui::GetFrameCount();
			if (frame - active_context()->last_cleanup_frame < 600)
			{
				return;
			}

			active_context()->last_cleanup_frame = frame;
			for (auto it = g_anim.begin(); it != g_anim.end();)
			{
				if (frame - it->second.last_frame > 1200)
				{
					it = g_anim.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		ImU32 col_u32(const ImVec4& color, float alpha_mul = 1.0f)
		{
			ImVec4 c = color;
			c.w *= active_context()->style.alpha * active_context()->draw_alpha * alpha_mul;
			return ImGui::GetColorU32(c);
		}

		persistent_window* find_window(const char* name)
		{
			if (name == nullptr)
			{
				return nullptr;
			}
			for (persistent_window& window : active_context()->windows)
			{
				if (window.name == name)
				{
					return &window;
				}
			}
			return nullptr;
		}

		ImVec2 to_imvec(const Vec2& v)
		{
			return ImVec2(v.x, v.y);
		}

		Vec2 to_vec(const ImVec2& v)
		{
			return Vec2(v.x, v.y);
		}

		ImVec4 to_imvec(const Vec4& v)
		{
			return ImVec4(v.x, v.y, v.z, v.w);
		}

		Vec4 to_vec(const ImVec4& v)
		{
			return Vec4(v.x, v.y, v.z, v.w);
		}

		ImGuiID to_imgui_id(ID value)
		{
			return static_cast<ImGuiID>(value.value());
		}

		ID to_ui_id(ImGuiID value)
		{
			return ID(static_cast<ID::ValueType>(value));
		}

		ImGuiWindowFlags to_imgui_window_flags(WindowFlags value)
		{
			return static_cast<ImGuiWindowFlags>(value);
		}

		ImGuiInputTextFlags to_imgui_input_text_flags(InputTextFlags value)
		{
			return static_cast<ImGuiInputTextFlags>(value);
		}

		ImGuiComboFlags to_imgui_combo_flags(ComboFlags value)
		{
			return static_cast<ImGuiComboFlags>(value);
		}

		ImGuiSelectableFlags to_imgui_selectable_flags(SelectableFlags value)
		{
			return static_cast<ImGuiSelectableFlags>(value);
		}

		ImGuiColorEditFlags to_imgui_color_edit_flags(ColorEditFlags value)
		{
			return static_cast<ImGuiColorEditFlags>(value);
		}

		ImGuiTableFlags to_imgui_table_flags(TableFlags value)
		{
			return static_cast<ImGuiTableFlags>(value);
		}

		ImGuiTableColumnFlags to_imgui_table_column_flags(TableColumnFlags value)
		{
			return static_cast<ImGuiTableColumnFlags>(value);
		}

		ImGuiTableRowFlags to_imgui_table_row_flags(TableRowFlags value)
		{
			return static_cast<ImGuiTableRowFlags>(value);
		}

		ImGuiDragDropFlags to_imgui_drag_drop_flags(DragDropFlags value)
		{
			return static_cast<ImGuiDragDropFlags>(value);
		}

		ImGuiCond to_imgui_cond(Condition value)
		{
			return static_cast<ImGuiCond>(value);
		}

		ImGuiKey to_imgui_key(Key value)
		{
			return static_cast<ImGuiKey>(value.value);
		}

		ImU32 col_u32(const Vec4& color, float alpha_mul = 1.0f)
		{
			ImVec4 c = to_imvec(color);
			c.w *= active_context()->style.alpha * active_context()->draw_alpha * alpha_mul;
			return ImGui::GetColorU32(c);
		}

		bool has_color(const Vec4& c)
		{
			return c.x != 0.0f || c.y != 0.0f || c.z != 0.0f || c.w != 0.0f;
		}

		Vec4 with_alpha(Vec4 c, float alpha)
		{
			c.w *= alpha;
			return c;
		}

		ImVec2 add(ImVec2 a, ImVec2 b)
		{
			return ImVec2(a.x + b.x, a.y + b.y);
		}

		ImVec2 sub(ImVec2 a, ImVec2 b)
		{
			return ImVec2(a.x - b.x, a.y - b.y);
		}

		ImVec2 mul(ImVec2 a, float v)
		{
			return ImVec2(a.x * v, a.y * v);
		}

		bool contains_case_insensitive(const char* text, const char* filter)
		{
			if (filter == nullptr || filter[0] == '\0')
			{
				return true;
			}
			if (text == nullptr)
			{
				return false;
			}
			String a(text);
			String b(filter);
			std::transform(a.begin(), a.end(), a.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			std::transform(b.begin(), b.end(), b.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return a.find(b) != String::npos;
		}

		bool is_modifier_key(ui::key key)
		{
			return key == ui::key::left_ctrl || key == ui::key::right_ctrl || key == ui::key::left_shift ||
			       key == ui::key::right_shift || key == ui::key::left_alt || key == ui::key::right_alt ||
			       key == ui::key::left_super || key == ui::key::right_super;
		}

		bool keybind_mods_match(const ui::keybind& binding)
		{
			ImGuiIO& io = ImGui::GetIO();
			return (!binding.ctrl || io.KeyCtrl) && (!binding.shift || io.KeyShift) && (!binding.alt || io.KeyAlt) &&
			       (!binding.super || io.KeySuper);
		}

		bool point_in_rect(ImVec2 point, ImVec2 min, ImVec2 max)
		{
			return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y;
		}

		bool consume_pending_modal(const char* name)
		{
			if (name == nullptr)
			{
				return false;
			}

			for (auto it = g_pending_modals.begin(); it != g_pending_modals.end(); ++it)
			{
				if (*it == name)
				{
					g_pending_modals.erase(it);
					return true;
				}
			}
			return false;
		}

		bool consume_pending_popup(const char* name)
		{
			if (name == nullptr)
			{
				return false;
			}

			for (auto it = g_pending_popups.begin(); it != g_pending_popups.end(); ++it)
			{
				if (*it == name)
				{
					g_pending_popups.erase(it);
					return true;
				}
			}
			return false;
		}

		const char* visible_label(const char* label)
		{
			if (label == nullptr)
			{
				return "";
			}
			const char* marker = std::strstr(label, "##");
			return marker == label ? "" : label;
		}

		void text_v(const Vec4& color, const char* fmt, va_list args)
		{
			char buffer[2048];
			std::vsnprintf(buffer, sizeof(buffer), fmt, args);
			ImGui::PushStyleColor(ImGuiCol_Text, to_imvec(color));
			ImGui::TextUnformatted(buffer);
			ImGui::PopStyleColor();
		}

		ImVec2 default_item_size(const char* label, ImVec2 requested, float min_width = 0.0f)
		{
			ImVec2 text_size = ImGui::CalcTextSize(label, nullptr, true);
			ImVec2 size      = requested;
			if (size.x <= 0.0f)
			{
				size.x = std::max(min_width, text_size.x + active_context()->style.padding * 2.0f);
			}
			if (size.y <= 0.0f)
			{
				size.y = active_context()->style.frame_height;
			}
			return size;
		}

		void draw_chevron(ImDrawList* draw, ImVec2 center, float size, float t, ImU32 color)
		{
			const float angle      = Math::lerp(0.0f, 1.5707963f, apply_ease(t));
			const ImVec2 points[3] = {ImVec2(-0.35f, -0.50f), ImVec2(0.35f, 0.00f), ImVec2(-0.35f, 0.50f)};
			ImVec2 out[3];
			const float cs = std::cos(angle);
			const float sn = std::sin(angle);
			for (int i = 0; i < 3; ++i)
			{
				const float x = points[i].x * size;
				const float y = points[i].y * size;
				out[i]        = ImVec2(center.x + x * cs - y * sn, center.y + x * sn + y * cs);
			}
			draw->AddTriangleFilled(out[0], out[1], out[2], color);
		}

		bool animated_row(const char* label, const char* icon, const char* right_text, bool selected, bool disabled, ImVec2 size,
		                  Vec4 accent, bool draw_arrow, float arrow_t)
		{
			cleanup_states();
			ImGui::PushID(label);
			const ImGuiID id = ImGui::GetID("row");
			AnimState& anim  = state_for(id);
			size.x           = size.x <= 0.0f ? ImGui::GetContentRegionAvail().x : size.x;
			size.y           = size.y <= 0.0f ? active_context()->style.frame_height : size.y;

			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("##row_button", size);
			const bool hovered = !disabled && ImGui::IsItemHovered();
			const bool active  = !disabled && ImGui::IsItemActive();
			const bool clicked = !disabled && ImGui::IsItemClicked();

			anim.hover    = approach(anim.hover, hovered ? 1.0f : 0.0f, active_context()->style.animation_speed);
			anim.active   = approach(anim.active, active ? 1.0f : 0.0f, active_context()->style.animation_speed * 1.6f);
			anim.selected = approach(anim.selected, selected ? 1.0f : 0.0f, active_context()->style.animation_speed);

			const Vec4 base = Math::lerp(active_context()->style.colors.panel, active_context()->style.colors.background_hovered,
			                             anim.hover);
			const Vec4 sel  = has_color(accent) ? accent : active_context()->style.colors.accent;
			Vec4 bg         = Math::lerp(base, with_alpha(sel, 0.22f), anim.selected);
			bg              = Math::lerp(bg, active_context()->style.colors.background_active, anim.active);

			ImDrawList* draw = ImGui::GetWindowDrawList();
			const ImVec2 max = add(pos, size);
			if (anim.hover > 0.01f || anim.selected > 0.01f || anim.active > 0.01f)
			{
				draw->AddRectFilled(pos, max, col_u32(bg), active_context()->style.rounding);
			}
			if (anim.selected > 0.01f)
			{
				draw->AddRectFilled(pos, ImVec2(pos.x + 3.0f, max.y), col_u32(sel, anim.selected),
				                    active_context()->style.rounding, ImDrawFlags_RoundCornersLeft);
			}

			float x        = pos.x + active_context()->style.padding;
			const float cy = pos.y + size.y * 0.5f;
			if (draw_arrow)
			{
				draw_chevron(draw, ImVec2(x + 5.0f, cy), 10.0f, arrow_t,
				             col_u32(Math::lerp(active_context()->style.colors.text_muted, sel, anim.hover + anim.selected)));
				x += 18.0f;
			}
			if (icon != nullptr && icon[0] != '\0')
			{
				draw->AddText(ImVec2(x, pos.y + (size.y - ImGui::GetTextLineHeight()) * 0.5f),
				              col_u32(Math::lerp(active_context()->style.colors.text_muted, sel, anim.hover)), icon);
				x += ImGui::CalcTextSize(icon).x + 7.0f;
			}

			const Vec4 text_col = disabled ? active_context()->style.colors.text_disabled
			                               : Math::lerp(active_context()->style.colors.text, sel, anim.selected * 0.35f);
			draw->AddText(ImVec2(x, pos.y + (size.y - ImGui::GetTextLineHeight()) * 0.5f), col_u32(text_col),
			              visible_label(label));
			if (right_text != nullptr && right_text[0] != '\0')
			{
				const ImVec2 rt = ImGui::CalcTextSize(right_text);
				draw->AddText(ImVec2(max.x - rt.x - active_context()->style.padding, pos.y + (size.y - rt.y) * 0.5f),
				              col_u32(active_context()->style.colors.text_muted), right_text);
			}

			ImGui::PopID();
			return clicked;
		}

		Vec4 notification_color(ui::notification_kind kind)
		{
			switch (kind)
			{
				case ui::notification_kind::success: return active_context()->style.colors.success;
				case ui::notification_kind::warning: return active_context()->style.colors.warning;
				case ui::notification_kind::error: return active_context()->style.colors.error;
				case ui::notification_kind::info:
				default: return active_context()->style.colors.accent;
			}
		}

		void push_menu_bar_colors()
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, active_context()->style.rounding * 0.55f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
			                    ImVec2(active_context()->style.spacing, active_context()->style.spacing * 0.75f));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(active_context()->style.padding, 6.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, to_imvec(active_context()->style.colors.text));
			ImGui::PushStyleColor(ImGuiCol_Header, to_imvec(with_alpha(active_context()->style.colors.accent, 0.16f)));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
			                      to_imvec(with_alpha(active_context()->style.colors.accent_hovered, 0.24f)));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive,
			                      to_imvec(with_alpha(active_context()->style.colors.accent_active, 0.30f)));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
			                      to_imvec(with_alpha(active_context()->style.colors.accent_hovered, 0.18f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive,
			                      to_imvec(with_alpha(active_context()->style.colors.accent_active, 0.24f)));
		}

		void pop_menu_bar_colors()
		{
			ImGui::PopStyleColor(7);
			ImGui::PopStyleVar(3);
		}

		void push_menu_popup_colors()
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, active_context()->style.rounding);
			ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, active_context()->style.rounding);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, active_context()->style.rounding * 0.55f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
			                    ImVec2(active_context()->style.spacing, active_context()->style.spacing * 0.6f));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(active_context()->style.padding, 6.0f));
			ImGui::PushStyleColor(ImGuiCol_PopupBg, to_imvec(active_context()->style.colors.panel));
			ImGui::PushStyleColor(ImGuiCol_Border, to_imvec(active_context()->style.colors.border));
			ImGui::PushStyleColor(ImGuiCol_Text, to_imvec(active_context()->style.colors.text));
			ImGui::PushStyleColor(ImGuiCol_Header, to_imvec(with_alpha(active_context()->style.colors.accent, 0.16f)));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
			                      to_imvec(with_alpha(active_context()->style.colors.accent_hovered, 0.24f)));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive,
			                      to_imvec(with_alpha(active_context()->style.colors.accent_active, 0.30f)));
		}

		void pop_menu_popup_colors()
		{
			ImGui::PopStyleColor(6);
			ImGui::PopStyleVar(5);
		}

		void draw_menu_bar_background()
		{
			ImDrawList* draw        = ImGui::GetWindowDrawList();
			const ImGuiStyle& style = ImGui::GetStyle();
			const ImVec2 cursor     = ImGui::GetCursorScreenPos();
			const ImVec2 window_pos = ImGui::GetWindowPos();
			const float y           = cursor.y - style.FramePadding.y;
			const ImVec2 min(window_pos.x, y);
			const ImVec2 max(window_pos.x + ImGui::GetWindowWidth(), y + ImGui::GetFrameHeight());
			draw->AddRectFilled(min, max, col_u32(active_context()->style.colors.panel), 0.0f);
			draw->AddLine(ImVec2(min.x, max.y), max, col_u32(active_context()->style.colors.border));
		}

		void push_window_styles(bool modal)
		{
			const Vec4 bg       = modal ? active_context()->style.colors.panel : active_context()->style.colors.background;
			const Vec4 title_bg = Math::lerp(active_context()->style.colors.background_active,
			                                 active_context()->style.colors.accent_active, 0.35f);
			const Vec4 title_bg_active =
			        Math::lerp(active_context()->style.colors.background_active, active_context()->style.colors.accent, 0.24f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, active_context()->style.rounding);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, active_context()->style.border_size);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
			                    ImVec2(active_context()->style.padding, active_context()->style.padding));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(active_context()->style.padding, 7.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, active_context()->style.alpha);
			ImGui::PushStyleColor(ImGuiCol_WindowBg, to_imvec(bg));
			ImGui::PushStyleColor(ImGuiCol_PopupBg, to_imvec(bg));
			ImGui::PushStyleColor(ImGuiCol_Border, to_imvec(active_context()->style.colors.border));
			ImGui::PushStyleColor(ImGuiCol_TitleBg, to_imvec(title_bg));
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, to_imvec(title_bg_active));
			ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, to_imvec(title_bg));
			ImGui::PushStyleColor(ImGuiCol_Text, to_imvec(active_context()->style.colors.text));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
			                      to_imvec(with_alpha(active_context()->style.colors.accent_hovered, 0.24f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive,
			                      to_imvec(with_alpha(active_context()->style.colors.accent_active, 0.30f)));
		}

		void pop_window_styles()
		{
			ImGui::PopStyleColor(10);
			ImGui::PopStyleVar(5);
		}

		void render_notifications()
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			const ImVec2 origin(viewport->WorkPos.x + viewport->WorkSize.x - 16.0f, viewport->WorkPos.y + 16.0f);
			float target_y   = origin.y;
			ImDrawList* draw = ImGui::GetForegroundDrawList(viewport);

			for (auto it = g_notifications.begin(); it != g_notifications.end();)
			{
				it->age += dt();
				const float in_t   = Math::clamp(it->age / 0.25f, 0.f, 1.f);
				const float out_t  = Math::clamp((it->duration - it->age) / 0.35f, 0.f, 1.f);
				const float alpha  = Math::clamp(Math::min(in_t, out_t), 0.f, 1.f);
				const float slide  = (1.0f - apply_ease(in_t)) * 24.0f;
				const float width  = 320.0f;
				const float height = (it->title.empty() ? 52.0f : 70.0f) + (it->action_label.empty() ? 0.0f : 28.0f);
				float& animated_y  = g_notification_y[it->id];
				if (animated_y == 0.0f)
				{
					animated_y = target_y;
				}
				animated_y = approach(animated_y, target_y, active_context()->style.animation_speed);
				const ImVec2 max(origin.x - slide, animated_y + height);
				const ImVec2 min(max.x - width, animated_y);
				const Vec4 accent = notification_color(it->kind);
				draw->AddRectFilled(min, max, col_u32(active_context()->style.colors.panel, alpha * 0.96f),
				                    active_context()->style.rounding);
				draw->AddRect(min, max, col_u32(active_context()->style.colors.border, alpha), active_context()->style.rounding);
				draw->AddRectFilled(min, ImVec2(min.x + 4.0f, max.y), col_u32(accent, alpha), active_context()->style.rounding,
				                    ImDrawFlags_RoundCornersLeft);
				float tx = min.x + 16.0f;
				float ty = min.y + 12.0f;
				if (!it->title.empty())
				{
					draw->AddText(ImVec2(tx, ty), col_u32(active_context()->style.colors.text, alpha), it->title.c_str());
					ty += ImGui::GetTextLineHeight() + 5.0f;
				}
				draw->AddText(ImVec2(tx, ty), col_u32(active_context()->style.colors.text_muted, alpha), it->message.c_str());
				if (!it->action_label.empty())
				{
					const ImVec2 button_size(ImGui::CalcTextSize(it->action_label.c_str()).x + 18.0f, 22.0f);
					const ImVec2 button_min(max.x - button_size.x - 12.0f, max.y - button_size.y - 10.0f);
					const ImVec2 button_max = add(button_min, button_size);
					const bool hovered      = ImGui::IsMouseHoveringRect(button_min, button_max);
					draw->AddRectFilled(button_min, button_max,
					                    col_u32(hovered ? active_context()->style.colors.accent_hovered
					                                    : active_context()->style.colors.accent,
					                            alpha),
					                    5.0f);
					draw->AddText(ImVec2(button_min.x + 9.0f, button_min.y + 3.0f),
					              col_u32(active_context()->style.colors.text, alpha), it->action_label.c_str());

					if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						if (it->action)
						{
							it->action();
						}
						g_notification_y.erase(it->id);
						it = g_notifications.erase(it);
						continue;
					}
				}
				target_y += height + 10.0f;
				if (it->age > it->duration)
				{
					g_notification_y.erase(it->id);
					it = g_notifications.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		void render_windows()
		{
			for (persistent_window& window : active_context()->windows)
			{
				if (!window.open)
				{
					continue;
				}

				bool open                          = true;
				active_context()->rendering_window = &window;
				const bool visible                 = begin_window(window.name.c_str(), &open, window.flags);
				active_context()->rendering_window = nullptr;
				window.open                        = open;
				if (visible && window.content)
				{
					window.content();
				}
				end_window();
			}
		}
	}// namespace

	void initialize()
	{
		active_context()->style = style{};
	}

	void shutdown()
	{
		active_context()->style_stack.clear();
		g_anim.clear();
		g_open.clear();
		g_hover_time.clear();
		g_notification_y.clear();
		g_active_combo           = 0;
		g_active_combo_field_min = ImVec2(0.0f, 0.0f);
		g_active_combo_field_max = ImVec2(0.0f, 0.0f);
		g_active_combo_popup_min = ImVec2(0.0f, 0.0f);
		g_active_combo_popup_max = ImVec2(0.0f, 0.0f);
		g_menu_bar_style_depth   = 0;
		g_menu_popup_style_depth = 0;
		g_menu_alpha_depth       = 0;
		g_table_style_depth      = 0;
		g_tree_indent_stack.clear();
		g_tree_stack.clear();
		g_area_stack.clear();
		g_disabled_alpha_stack.clear();
		g_pending_modals.clear();
		g_pending_popups.clear();
		g_notifications.clear();
		active_context()->windows.clear();
		active_context()->keybind_capture = 0;
		active_context()->draw_alpha      = 1.0f;
	}

	static void register_font(Buffer& buffer, const ImFontConfig& config, const ImWchar* range, f32 size)
	{
		auto& io = ImGui::GetIO();

		void* memory = ImGui::MemAlloc(buffer.size());
		memcpy(memory, buffer.data(), buffer.size());
		io.Fonts->AddFontFromMemoryTTF(memory, buffer.size(), size, &config, range);
	}

	static void register_font(const Path& path, const ImFontConfig& config, const ImWchar* range)
	{
		FileReader reader(path);
		trinex_verify(reader.is_open());

		Buffer buffer = reader.read_buffer();
		register_font(buffer, config, range, Settings::Editor::normal_font_size);
	}

	Context* create_context(Trinex::Window* window)
	{
		Context* ctx = trx_new Context();
		ctx->window  = window;
		ctx->context = ImGui::CreateContext();

		UI::Backend::imgui_init(window, ctx->context);

		// Load fonts

		auto& io = ImGui::GetIO();
		{
			const ImWchar icons_ranges[] = {
			        0x0020,      0x00FF,// ASCII + Latin
			        0x2000,      0x206F,// punctuation
			        0x2190,      0x21FF,// arrows
			        0x2700,      0x27BF,// dingbats
			        ICON_MIN_LC, ICON_MAX_LC, 0,
			};

			ImFontConfig cfg;
			cfg.FontDataOwnedByAtlas = true;

			register_font(Settings::Editor::font_path, cfg, io.Fonts->GetGlyphRangesCyrillic());
			register_font("[content]:/TrinexEditor/fonts/Lucide/lucide.ttf", cfg, icons_ranges);
		}

		io.Fonts->Build();
		io.FontDefault = io.Fonts->Fonts[0];
		io.IniFilename = nullptr;
		io.LogFilename = nullptr;
		return ctx;
	}

	void destroy_context(Context* context)
	{
		UI::Backend::imgui_shutdown(context->window, context->context);
		ImGui::DestroyContext(context->context);
		trx_delete context;
	}

	void begin_frame(Context* context)
	{
		trinex_assert(context);
		g_context = context;

		ImGui::SetCurrentContext(context->context);
		UI::Backend::imgui_new_frame(context->window);
		ImGui::NewFrame();
	}

	void end_frame()
	{
		trinex_assert(g_context);

		render_windows();
		render_notifications();
		ImGui::Render();

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
		}

		{
			auto viewport           = g_context->window->render_viewport();
			RHISwapchain* swapchain = viewport->swapchain();

			RHIContext* ctx = RHIContextPool::global_instance()->begin_context();
			{
				auto texture = swapchain->as_texture();
				auto rtv     = texture->as_rtv();

				ctx->barrier(texture, RHIAccess::TransferDst);
				ctx->clear_rtv(rtv, 0.f, 0.f, 0.f, 1.f);
				ctx->barrier(texture, RHIAccess::RTV);

				UI::Backend::imgui_render(ctx, ImGui::GetDrawData());

				ctx->barrier(texture, RHIAccess::PresentSrc);
			}

			RHIContextPool::global_instance()->end_context(ctx, swapchain->acquire_semaphore(), swapchain->present_semaphore());
			RHI::instance()->present(swapchain);
		}

		ImGui::SetCurrentContext(nullptr);
	}

	Context* context()
	{
		return g_context;
	}

	void context(Context* value) {}

	style& get_style()
	{
		return active_context()->style;
	}

	void set_style(const style& value)
	{
		active_context()->style = value;
	}

	void push_style(const style& value)
	{
		active_context()->style_stack.push_back(active_context()->style);
		active_context()->style = value;
	}

	void pop_style()
	{
		if (!active_context()->style_stack.empty())
		{
			active_context()->style = active_context()->style_stack.back();
			active_context()->style_stack.pop_back();
		}
	}

	DisabledScope::DisabledScope(bool disabled)
	{
		begin_disabled(disabled);
	}

	DisabledScope::~DisabledScope()
	{
		end_disabled();
	}

	StyleScope::StyleScope(const style& value)
	{
		push_style(value);
	}

	StyleScope::~StyleScope()
	{
		pop_style();
	}

	IdScope::IdScope(const char* id)
	{
		push_id(id);
	}

	IdScope::IdScope(int id)
	{
		push_id(id);
	}

	IdScope::~IdScope()
	{
		pop_id();
	}

	float apply_ease(float t, ease mode)
	{
		t = Math::clamp(t, 0.f, 1.f);
		switch (mode)
		{
			case ease::linear: return t;
			case ease::in_quad: return t * t;
			case ease::out_quad: return 1.0f - (1.0f - t) * (1.0f - t);
			case ease::in_out_quad: return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f;
			case ease::out_back:
			{
				const float c1 = 1.70158f;
				const float c3 = c1 + 1.0f;
				return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
			}
			case ease::out_cubic:
			default: return 1.0f - std::pow(1.0f - t, 3.0f);
		}
	}

	float animate_float(ID id, float target, float speed)
	{
		AnimState& anim = state_for(to_imgui_id(id));
		anim.value      = approach(anim.value, target, speed < 0.0f ? active_context()->style.animation_speed : speed);
		return anim.value;
	}

	Vec2 animate_vec2(ID id, const Vec2& target, float speed)
	{
		AnimState& anim = state_for(to_imgui_id(id));
		const float s   = speed < 0.0f ? active_context()->style.animation_speed : speed;
		anim.value      = approach(anim.value, target.x, s);
		anim.extra      = approach(anim.extra, target.y, s);
		return Vec2(anim.value, anim.extra);
	}

	Vec4 animate_color(ID id, const Vec4& target, float speed)
	{
		AnimState& anim = state_for(to_imgui_id(id));
		const float s   = speed < 0.0f ? active_context()->style.animation_speed : speed;
		anim.hover      = approach(anim.hover, target.x, s);
		anim.active     = approach(anim.active, target.y, s);
		anim.focus      = approach(anim.focus, target.z, s);
		anim.selected   = approach(anim.selected, target.w, s);
		return Vec4(anim.hover, anim.active, anim.focus, anim.selected);
	}

	void reset_animation(ID id)
	{
		g_anim.erase(to_imgui_id(id));
	}

	void clear_animations()
	{
		g_anim.clear();
	}

	void push_id(const char* id)
	{
		ImGui::PushID(id);
	}

	void push_id(int id)
	{
		ImGui::PushID(id);
	}

	void pop_id()
	{
		ImGui::PopID();
	}

	ID id(const char* id)
	{
		return to_ui_id(ImGui::GetID(id));
	}

	bool begin_window(const char* name, bool* open, WindowFlags flags)
	{
		push_window_styles(false);
		ActiveWindowScope scope;
		scope.external_open = open;
		scope.persistent    = active_context()->rendering_window;
		active_context()->active_window_stack.push_back(scope);
		const bool visible = ImGui::Begin(name, open, to_imgui_window_flags(flags));
		return visible;
	}

	void end_window()
	{
		ImGui::End();
		if (!active_context()->active_window_stack.empty())
		{
			active_context()->active_window_stack.pop_back();
		}
		pop_window_styles();
	}

	void create_window(const char* name, const Function<void()>& content, WindowFlags flags)
	{
		create_window(name, true, content, flags);
	}

	void create_window(const char* name, bool open, const Function<void()>& content, WindowFlags flags)
	{
		if (name == nullptr || content == nullptr)
		{
			return;
		}
		if (persistent_window* window = find_window(name))
		{
			window->flags   = flags;
			window->open    = open;
			window->content = content;
			return;
		}

		persistent_window window;
		window.name    = name;
		window.flags   = flags;
		window.open    = open;
		window.content = content;
		active_context()->windows.push_back(std::move(window));
	}

	bool is_window_open(const char* name)
	{
		if (persistent_window* window = find_window(name))
		{
			return window->open;
		}
		return false;
	}

	void open_window(const char* name)
	{
		if (persistent_window* window = find_window(name))
		{
			window->open = true;
		}
	}

	void close_window()
	{
		if (active_context()->active_window_stack.empty())
		{
			return;
		}

		ActiveWindowScope& scope = active_context()->active_window_stack.back();
		if (scope.external_open != nullptr)
		{
			*scope.external_open = false;
		}
		if (scope.persistent != nullptr)
		{
			scope.persistent->open = false;
		}
	}

	void close_window(const char* name)
	{
		if (persistent_window* window = find_window(name))
		{
			window->open = false;
		}
	}

	bool begin_panel(const char* id, const panel_options& options)
	{
		const Vec4 bg = has_color(options.background_color) ? options.background_color : active_context()->style.colors.panel;
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,
		                    options.rounding >= 0.0f ? options.rounding : active_context()->style.rounding);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, options.border ? active_context()->style.border_size : 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
		                    ImVec2(active_context()->style.padding, active_context()->style.padding));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, options.background ? to_imvec(bg) : ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Border, to_imvec(active_context()->style.colors.border));
		return ImGui::BeginChild(id, to_imvec(options.size), options.border, ImGuiWindowFlags_None);
	}

	void end_panel()
	{
		ImGui::EndChild();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(3);
	}

	bool begin_child_panel(const char* id, const Vec2& size, const panel_options& options)
	{
		panel_options copy = options;
		copy.size          = size;
		return begin_panel(id, copy);
	}

	void end_child_panel()
	{
		end_panel();
	}

	bool begin_group_panel(const char* label, const Vec2& size, const panel_options& options)
	{
		ImGui::BeginGroup();
		if (label != nullptr && label[0] != '\0')
		{
			text_muted("%s", label);
		}
		return begin_child_panel(label, size, options);
	}

	void end_group_panel()
	{
		end_child_panel();
		ImGui::EndGroup();
	}

	void separator()
	{
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Separator, to_imvec(active_context()->style.colors.border));
		ImGui::Separator();
		ImGui::PopStyleColor();
		ImGui::Spacing();
	}

	void spacing(float amount)
	{
		if (amount < 0.0f)
		{
			ImGui::Spacing();
			return;
		}
		ImGui::Dummy(ImVec2(0.0f, amount));
	}

	void same_line(float offset_from_start_x, float spacing_value)
	{
		ImGui::SameLine(offset_from_start_x, spacing_value);
	}

	void begin_disabled(bool disabled)
	{
		ImGui::BeginDisabled(disabled);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * (disabled ? 0.55f : 1.0f));
		g_disabled_alpha_stack.push_back(active_context()->draw_alpha);
		if (disabled)
		{
			active_context()->draw_alpha *= 0.55f;
		}
	}

	void end_disabled()
	{
		if (!g_disabled_alpha_stack.empty())
		{
			active_context()->draw_alpha = g_disabled_alpha_stack.back();
			g_disabled_alpha_stack.pop_back();
		}
		ImGui::PopStyleVar();
		ImGui::EndDisabled();
	}

	bool begin_animated_area(const char* id_label, bool visible)
	{
		ImGui::PushID(id_label);
		const ImGuiID id  = ImGui::GetID("animated_area");
		AnimState& anim   = state_for(id);
		anim.open         = approach(anim.open, visible ? 1.0f : 0.0f, active_context()->style.animation_speed);
		const bool render = visible || anim.open > 0.001f;
		if (!render)
		{
			ImGui::PopID();
			return false;
		}

		const float eased          = apply_ease(anim.open, ease::in_out_quad);
		const ImVec2 start         = ImGui::GetCursorScreenPos();
		const float visible_height = std::max(0.0f, anim.extra) * eased;
		const ImVec2 clip_min(start.x - 2.0f, start.y);
		const ImVec2 clip_max(start.x + ImGui::GetContentRegionAvail().x + 2.0f, start.y + visible_height);

		ImGui::PushClipRect(clip_min, clip_max, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * eased);

		area_context context;
		context.id                  = id;
		context.content_start       = start;
		context.previous_draw_alpha = active_context()->draw_alpha;
		g_area_stack.push_back(context);
		active_context()->draw_alpha *= eased;
		return true;
	}

	void end_animated_area()
	{
		if (g_area_stack.empty())
		{
			return;
		}

		area_context context = g_area_stack.back();
		g_area_stack.pop_back();
		const ImVec2 end            = ImGui::GetCursorScreenPos();
		const float measured_height = std::max(0.0f, end.y - context.content_start.y);
		AnimState& anim             = state_for(context.id);
		if (measured_height > 0.0f)
		{
			anim.extra = measured_height;
		}

		const float visible_height   = std::max(anim.extra, measured_height) * apply_ease(anim.open, ease::in_out_quad);
		active_context()->draw_alpha = context.previous_draw_alpha;
		ImGui::PopStyleVar();
		ImGui::PopClipRect();
		ImGui::SetCursorScreenPos(ImVec2(context.content_start.x, context.content_start.y + visible_height));
		ImGui::PopID();
	}

	void animated_area(const char* id, bool visible, const Function<void()>& content)
	{
		if (begin_animated_area(id, visible))
		{
			if (content)
			{
				content();
			}
			end_animated_area();
		}
	}

	bool begin_scroll_area(const char* id, const Vec2& size, bool border, WindowFlags flags)
	{
		return ImGui::BeginChild(id, to_imvec(size), border, to_imgui_window_flags(flags));
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

	void text(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		text_v(active_context()->style.colors.text, fmt, args);
		va_end(args);
	}

	void text_muted(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		text_v(active_context()->style.colors.text_muted, fmt, args);
		va_end(args);
	}

	void text_colored(const Vec4& color, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		text_v(color, fmt, args);
		va_end(args);
	}

	void label(const char* label_text, const char* value)
	{
		text_muted("%s", label_text);
		if (value != nullptr)
		{
			ImGui::SameLine();
			text("%s", value);
		}
	}

	void help_marker(const char* description)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
		{
			tooltip(description);
		}
	}

	void tooltip(const char* value)
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(value);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}

	void tooltip_delayed(const char* value, float delay)
	{
		const ImGuiID id = ImGui::GetItemID();
		float& time      = g_hover_time[id];
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

	void tooltip_if_hovered(const char* value, float delay)
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

	void help_tooltip(const char* description)
	{
		help_marker(description);
	}

	void image(RHITexture* texture, const Vec2& size, const ImageOptions& options)
	{
		if (texture == nullptr)
		{
			ImGui::Dummy(to_imvec(size));
			return;
		}

		RHISampler* sampler  = options.sampler ? options.sampler : RHIPointSampler::static_sampler();
		const float rounding = options.rounding >= 0.0f ? options.rounding : active_context()->style.rounding;
		const float padding  = std::max(0.0f, options.padding);
		const ImVec2 image_size(std::max(0.0f, size.x), std::max(0.0f, size.y));
		const ImVec2 frame_size(image_size.x + padding * 2.0f, image_size.y + padding * 2.0f);
		const ImVec2 pos = ImGui::GetCursorScreenPos();

		ImGui::Dummy(frame_size);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		const ImVec2 max = add(pos, frame_size);
		const ImVec2 image_min(pos.x + padding, pos.y + padding);
		const ImVec2 image_max(image_min.x + image_size.x, image_min.y + image_size.y);
		const float image_rounding = std::max(0.0f, rounding - padding * 0.5f);

		Vec4 frame_bg     = has_color(options.background_color) ? options.background_color : active_context()->style.colors.panel;
		Vec4 frame_border = has_color(options.border_color) ? options.border_color : active_context()->style.colors.border;

		if (options.background)
		{
			draw->AddRectFilled(pos, max, col_u32(frame_bg), rounding);
		}

		draw->AddImageRounded(ImTextureID(texture, sampler), image_min, image_max, to_imvec(options.uv0), to_imvec(options.uv1),
		                      col_u32(options.tint), image_rounding);

		if (options.border)
		{
			draw->AddRect(pos, max, col_u32(frame_border), rounding, 0, active_context()->style.border_size);
		}
	}

	bool image_button(const char* id_text, RHITexture* texture, const Vec2& size, const ImageOptions& options)
	{
		if (texture == nullptr)
		{
			ImGui::Dummy(to_imvec(size));
			return false;
		}

		cleanup_states();
		if (id_text != nullptr)
		{
			ImGui::PushID(id_text);
		}
		else
		{
			ImGui::PushID(texture);
		}

		const ImGuiID id     = ImGui::GetID("image_button");
		AnimState& anim      = state_for(id);
		RHISampler* sampler  = options.sampler ? options.sampler : RHIPointSampler::static_sampler();
		const float rounding = options.rounding >= 0.0f ? options.rounding : active_context()->style.rounding;
		const float padding  = std::max(0.0f, options.padding);
		const ImVec2 image_size(std::max(0.0f, size.x), std::max(0.0f, size.y));
		const ImVec2 frame_size(image_size.x + padding * 2.0f, image_size.y + padding * 2.0f);
		const ImVec2 pos = ImGui::GetCursorScreenPos();

		ImGui::InvisibleButton("##image_button", frame_size);

		const bool hovered = ImGui::IsItemHovered();
		const bool active  = ImGui::IsItemActive();
		const bool clicked = ImGui::IsItemClicked();

		anim.hover  = approach(anim.hover, hovered ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.active = approach(anim.active, active ? 1.0f : 0.0f, active_context()->style.animation_speed * 1.5f);

		const Vec4 accent = has_color(options.accent) ? options.accent : active_context()->style.colors.accent;
		Vec4 frame_bg     = has_color(options.background_color) ? options.background_color : active_context()->style.colors.panel;
		Vec4 frame_border = has_color(options.border_color) ? options.border_color : active_context()->style.colors.border;
		const Vec2 hover_scale = active_context()->style.hover_scale;
		const Vec2 scale(1.0f + anim.hover * hover_scale.x - anim.active * 0.05f,
		                 1.0f + anim.hover * hover_scale.y - anim.active * 0.05f);

		frame_bg     = Math::lerp(frame_bg, active_context()->style.colors.background_hovered, anim.hover * 0.55f);
		frame_bg     = Math::lerp(frame_bg, active_context()->style.colors.background_active, anim.active * 0.65f);
		frame_border = Math::lerp(frame_border, accent, anim.hover * 0.7f + anim.active * 0.3f);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		const ImVec2 center(pos.x + frame_size.x * 0.5f, pos.y + frame_size.y * 0.5f);
		const ImVec2 visual_frame_size(frame_size.x * scale.x, frame_size.y * scale.y);
		const ImVec2 visual_image_size(image_size.x * scale.x, image_size.y * scale.y);
		const ImVec2 min(center.x - visual_frame_size.x * 0.5f, center.y - visual_frame_size.y * 0.5f);
		const ImVec2 max(center.x + visual_frame_size.x * 0.5f, center.y + visual_frame_size.y * 0.5f);
		const ImVec2 image_min(center.x - visual_image_size.x * 0.5f, center.y - visual_image_size.y * 0.5f);
		const ImVec2 image_max(center.x + visual_image_size.x * 0.5f, center.y + visual_image_size.y * 0.5f);
		const float scale_rounding = std::min(scale.x, scale.y);
		const float image_rounding = std::max(0.0f, (rounding - padding * 0.5f) * scale_rounding);

		if (options.background)
		{
			draw->AddRectFilled(min, max, col_u32(frame_bg), rounding * scale_rounding);
		}

		draw->AddImageRounded(ImTextureID(texture, sampler), image_min, image_max, to_imvec(options.uv0), to_imvec(options.uv1),
		                      col_u32(options.tint), image_rounding);

		if (options.border)
		{
			draw->AddRect(min, max, col_u32(frame_border), rounding * scale_rounding, 0, active_context()->style.border_size);
		}

		ImGui::PopID();
		return clicked;
	}

	bool button(const char* label, const button_options& options)
	{
		cleanup_states();
		ImGui::PushID(label);
		const ImGuiID id  = ImGui::GetID("button");
		AnimState& anim   = state_for(id);
		const ImVec2 size = default_item_size(label, to_imvec(options.size), 72.0f);
		const ImVec2 pos  = ImGui::GetCursorScreenPos();

		if (options.disabled)
		{
			ImGui::BeginDisabled();
		}
		ImGui::InvisibleButton("##button", size);
		if (options.disabled)
		{
			ImGui::EndDisabled();
		}

		const bool hovered = !options.disabled && ImGui::IsItemHovered();
		const bool active  = !options.disabled && ImGui::IsItemActive();
		const bool clicked = !options.disabled && ImGui::IsItemClicked();
		anim.hover         = approach(anim.hover, hovered ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.active        = approach(anim.active, active ? 1.0f : 0.0f, active_context()->style.animation_speed * 1.5f);

		const Vec4 accent      = has_color(options.accent) ? options.accent : active_context()->style.colors.accent;
		Vec4 bg                = options.ghost ? Vec4(0, 0, 0, 0) : active_context()->style.colors.background_active;
		bg                     = Math::lerp(bg, active_context()->style.colors.background_hovered, anim.hover);
		bg                     = Math::lerp(bg, accent, anim.active * 0.65f);
		const Vec2 hover_scale = active_context()->style.hover_scale;
		const Vec2 scale(1.0f + anim.hover * hover_scale.x - anim.active * 0.05f,
		                 1.0f + anim.hover * hover_scale.y - anim.active * 0.05f);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		const ImVec2 center(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
		const ImVec2 visual_size(size.x * scale.x, size.y * scale.y);
		const ImVec2 min(center.x - visual_size.x * 0.5f, center.y - visual_size.y * 0.5f);
		const ImVec2 max(center.x + visual_size.x * 0.5f, center.y + visual_size.y * 0.5f);
		const float scale_rounding = std::min(scale.x, scale.y);
		if (!options.ghost || anim.hover > 0.01f || anim.active > 0.01f)
		{
			draw->AddRectFilled(min, max, col_u32(bg, options.disabled ? 0.45f : 1.0f),
			                    active_context()->style.rounding * scale_rounding);
		}
		draw->AddRect(
		        min, max,
		        col_u32(Math::lerp(active_context()->style.colors.border, accent, anim.hover), options.disabled ? 0.45f : 1.0f),
		        active_context()->style.rounding * scale_rounding, 0, active_context()->style.border_size);

		String full_label;
		if (options.icon != nullptr && options.icon[0] != '\0')
		{
			full_label = String(options.icon) + "  " + visible_label(label);
		}
		else
		{
			full_label = visible_label(label);
		}
		const ImVec2 ts = ImGui::CalcTextSize(full_label.c_str());
		const Vec4 tc   = options.disabled ? active_context()->style.colors.text_disabled
		                                   : Math::lerp(active_context()->style.colors.text, Vec4(1, 1, 1, 1), anim.hover * 0.35f);
		draw->AddText(ImVec2(center.x - ts.x * 0.5f, center.y - ts.y * 0.5f), col_u32(tc), full_label.c_str());
		ImGui::PopID();
		return clicked;
	}

	bool icon_button(const char* icon, const char* label, const button_options& options)
	{
		button_options copy = options;
		copy.icon           = icon;
		return button(label, copy);
	}

	bool small_button(const char* label)
	{
		button_options options;
		options.size = Vec2(0.0f, 24.0f);
		return button(label, options);
	}

	bool ghost_button(const char* label, const Vec2& size)
	{
		button_options options;
		options.size  = size;
		options.ghost = true;
		return button(label, options);
	}

	bool danger_button(const char* label, const Vec2& size)
	{
		button_options options;
		options.size   = size;
		options.accent = active_context()->style.colors.error;
		return button(label, options);
	}

	bool checkbox(const char* label, bool* value)
	{
		cleanup_states();
		ImGui::PushID(label);
		const ImGuiID id = ImGui::GetID("checkbox");
		AnimState& anim  = state_for(id);
		const float box  = 20.0f;
		const ImVec2 ts  = ImGui::CalcTextSize(visible_label(label));
		const ImVec2 size(box + active_context()->style.spacing + ts.x, std::max(box, active_context()->style.frame_height));
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("##checkbox", size);
		const bool clicked = ImGui::IsItemClicked();
		if (clicked && value != nullptr)
		{
			*value = !*value;
		}
		anim.hover  = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.active = approach(anim.active, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.value  = approach(anim.value, value != nullptr && *value ? 1.0f : 0.0f, active_context()->style.animation_speed);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		const ImVec2 box_pos(pos.x, pos.y + (size.y - box) * 0.5f);
		const Vec4 fill =
		        Math::lerp(active_context()->style.colors.background, active_context()->style.colors.accent, anim.value);
		draw->AddRectFilled(
		        box_pos, add(box_pos, ImVec2(box, box)),
		        col_u32(Math::lerp(fill, active_context()->style.colors.background_hovered, anim.hover * (1.0f - anim.value))),
		        active_context()->style.rounding * 0.55f);
		draw->AddRect(box_pos, add(box_pos, ImVec2(box, box)),
		              col_u32(Math::lerp(active_context()->style.colors.border, active_context()->style.colors.accent_hovered,
		                                 anim.hover + anim.value)),
		              active_context()->style.rounding * 0.55f, 0, active_context()->style.border_size);
		if (anim.value > 0.02f)
		{
			const float s  = apply_ease(anim.value, ease::out_back);
			const ImVec2 c = add(box_pos, ImVec2(box * 0.5f, box * 0.5f));
			draw->AddLine(ImVec2(c.x - 5.0f * s, c.y), ImVec2(c.x - 1.0f * s, c.y + 4.0f * s),
			              col_u32(active_context()->style.colors.text, anim.value), 2.0f);
			draw->AddLine(ImVec2(c.x - 1.0f * s, c.y + 4.0f * s), ImVec2(c.x + 6.0f * s, c.y - 5.0f * s),
			              col_u32(active_context()->style.colors.text, anim.value), 2.0f);
		}
		draw->AddText(ImVec2(pos.x + box + active_context()->style.spacing, pos.y + (size.y - ts.y) * 0.5f),
		              col_u32(active_context()->style.colors.text), visible_label(label));
		ImGui::PopID();
		return clicked;
	}

	bool toggle(const char* label, bool* value)
	{
		cleanup_states();
		ImGui::PushID(label);
		const ImGuiID id = ImGui::GetID("toggle");
		AnimState& anim  = state_for(id);
		const ImVec2 track(46.0f, 24.0f);
		const ImVec2 ts = ImGui::CalcTextSize(visible_label(label));
		const ImVec2 size(track.x + active_context()->style.spacing + ts.x,
		                  std::max(track.y, active_context()->style.frame_height));
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("##toggle", size);
		const bool clicked = ImGui::IsItemClicked();
		if (clicked && value != nullptr)
		{
			*value = !*value;
		}
		anim.hover = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.value = approach(anim.value, value != nullptr && *value ? 1.0f : 0.0f, active_context()->style.animation_speed);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		const ImVec2 p(pos.x, pos.y + (size.y - track.y) * 0.5f);
		const float toggle_t = apply_ease(anim.value, ease::in_out_quad);
		const Vec4 track_col = Math::lerp(Math::lerp(active_context()->style.colors.background_active,
		                                             active_context()->style.colors.background_hovered, anim.hover),
		                                  active_context()->style.colors.accent, toggle_t);
		draw->AddRectFilled(p, add(p, track), col_u32(track_col), track.y * 0.5f);
		const float knob_r = 9.0f + anim.hover;
		const float knob_x = Math::lerp(p.x + track.y * 0.5f, p.x + track.x - track.y * 0.5f, toggle_t);
		draw->AddCircleFilled(ImVec2(knob_x, p.y + track.y * 0.5f), knob_r, col_u32(active_context()->style.colors.text));
		draw->AddText(ImVec2(pos.x + track.x + active_context()->style.spacing, pos.y + (size.y - ts.y) * 0.5f),
		              col_u32(active_context()->style.colors.text), visible_label(label));
		ImGui::PopID();
		return clicked;
	}

	bool slider_float(const char* label, float* value, float min, float max, const char* format)
	{
		cleanup_states();
		ImGui::PushID(label);
		const ImGuiID id        = ImGui::GetID("slider_float");
		AnimState& anim         = state_for(id);
		const float width       = ImGui::CalcItemWidth();
		const ImVec2 label_size = ImGui::CalcTextSize(visible_label(label));
		const ImVec2 size(width, active_context()->style.frame_height);
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("##slider", size);
		bool changed = false;
		if (ImGui::IsItemActive() && value != nullptr && max > min)
		{
			const float mouse_t = Math::clamp((ImGui::GetIO().MousePos.x - pos.x) / size.x, 0.f, 1.f);
			const float next    = min + (max - min) * mouse_t;
			if (*value != next)
			{
				*value  = next;
				changed = true;
			}
		}
		const float target = value != nullptr && max > min ? Math::clamp((*value - min) / (max - min), 0.f, 1.f) : 0.0f;
		anim.value         = approach(anim.value, target, active_context()->style.animation_speed);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.active        = approach(anim.active, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);

		ImDrawList* draw  = ImGui::GetWindowDrawList();
		const float bar_h = 7.0f;
		const ImVec2 bar_min(pos.x, pos.y + (size.y - bar_h) * 0.5f);
		const ImVec2 bar_max(pos.x + size.x, bar_min.y + bar_h);
		draw->AddRectFilled(bar_min, bar_max, col_u32(active_context()->style.colors.background_active), bar_h * 0.5f);
		draw->AddRectFilled(bar_min, ImVec2(Math::lerp(bar_min.x, bar_max.x, anim.value), bar_max.y),
		                    col_u32(Math::lerp(active_context()->style.colors.accent,
		                                       active_context()->style.colors.accent_hovered, anim.hover)),
		                    bar_h * 0.5f);
		draw->AddCircleFilled(ImVec2(Math::lerp(bar_min.x, bar_max.x, anim.value), bar_min.y + bar_h * 0.5f),
		                      6.0f + anim.active * 2.0f, col_u32(active_context()->style.colors.text));
		if (label_size.x > 0.0f && std::strstr(label, "##") != label)
		{
			ImGui::SameLine();
			ImGui::TextUnformatted(visible_label(label));
		}
		if (value != nullptr && ImGui::IsItemHovered())
		{
			char buf[64];
			std::snprintf(buf, sizeof(buf), format, *value);
			ImGui::SetTooltip("%s", buf);
		}
		ImGui::PopID();
		return changed;
	}

	bool slider_int(const char* label, int* value, int min, int max, const char* format)
	{
		float v = value != nullptr ? static_cast<float>(*value) : 0.0f;
		(void) format;
		const bool changed = slider_float(label, &v, static_cast<float>(min), static_cast<float>(max), "%.0f");
		if (changed && value != nullptr)
		{
			*value = static_cast<int>(std::round(v));
		}
		return changed;
	}

	bool input_text(const char* label, char* buffer, size_t buffer_size, InputTextFlags flags)
	{
		cleanup_states();
		ImGui::PushID(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		anim.hover      = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, active_context()->style.rounding);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(active_context()->style.padding, 6.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, to_imvec(active_context()->style.colors.background));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, to_imvec(active_context()->style.colors.background_hovered));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, to_imvec(active_context()->style.colors.background_active));
		ImGui::PushStyleColor(ImGuiCol_Border, to_imvec(Math::lerp(active_context()->style.colors.border,
		                                                           active_context()->style.colors.accent, anim.focus)));
		const bool changed = ImGui::InputText(label, buffer, buffer_size, to_imgui_input_text_flags(flags));
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		ImGui::PopStyleColor(4);
		ImGui::PopStyleVar(2);
		ImGui::PopID();
		return changed;
	}

	bool search_input(const char* label, char* buffer, size_t buffer_size)
	{
		return input_text(label, buffer, buffer_size);
	}

	bool begin_combo(const char* label, const char* preview_value, ComboFlags flags)
	{
		(void) flags;
		cleanup_states();
		ImGui::PushID(label);
		const ImGuiID id = ImGui::GetID("combo_open");
		bool& open       = g_open[id];
		AnimState& anim  = state_for(id);

		const ImVec2 label_size = ImGui::CalcTextSize(visible_label(label));
		const float width       = ImGui::CalcItemWidth();
		const ImVec2 frame_size(width, active_context()->style.frame_height);
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("##combo_field", frame_size);
		const bool hovered              = ImGui::IsItemHovered();
		const bool active               = ImGui::IsItemActive();
		const ImVec2 mouse              = ImGui::GetIO().MousePos;
		const bool clicked_inside_field = point_in_rect(mouse, pos, add(pos, frame_size));
		const bool clicked_inside_popup =
		        g_active_combo == id && point_in_rect(mouse, g_active_combo_popup_min, g_active_combo_popup_max);
		if (ImGui::IsItemClicked())
		{
			const bool next_open = !open;
			if (next_open && g_active_combo != 0 && g_active_combo != id)
			{
				g_open[g_active_combo] = false;
			}
			open           = next_open;
			g_active_combo = open ? id : 0;
			if (open)
			{
				g_active_combo_field_min = pos;
				g_active_combo_field_max = add(pos, frame_size);
			}
		}
		else if (open && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !clicked_inside_field && !clicked_inside_popup)
		{
			open = false;
			if (g_active_combo == id)
			{
				g_active_combo = 0;
			}
		}
		else if (g_active_combo != 0 && g_active_combo != id)
		{
			open = false;
		}

		anim.hover  = approach(anim.hover, hovered ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.active = approach(anim.active, active ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.open   = approach(anim.open, open ? 1.0f : 0.0f, active_context()->style.animation_speed);
		if (open && anim.open > 0.995f)
		{
			anim.open = 1.0f;
		}
		else if (!open && anim.open < 0.005f)
		{
			anim.open = 0.0f;
		}
		const float open_t = apply_ease(anim.open, ease::in_out_quad);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		const Vec4 bg    = Math::lerp(Math::lerp(active_context()->style.colors.background,
		                                         active_context()->style.colors.background_hovered, anim.hover),
		                              active_context()->style.colors.background_active, anim.active);
		draw->AddRectFilled(pos, add(pos, frame_size), col_u32(bg), active_context()->style.rounding);
		draw->AddRect(pos, add(pos, frame_size),
		              col_u32(Math::lerp(active_context()->style.colors.border, active_context()->style.colors.accent, open_t)),
		              active_context()->style.rounding, 0, active_context()->style.border_size);
		const char* preview       = preview_value != nullptr ? preview_value : "";
		const ImVec2 preview_size = ImGui::CalcTextSize(preview);
		draw->AddText(ImVec2(pos.x + active_context()->style.padding, pos.y + (frame_size.y - preview_size.y) * 0.5f),
		              col_u32(active_context()->style.colors.text), preview);
		draw_chevron(draw, ImVec2(pos.x + frame_size.x - 16.0f, pos.y + frame_size.y * 0.5f), 9.0f, open_t,
		             col_u32(active_context()->style.colors.text_muted));

		if (label_size.x > 0.0f && std::strstr(label, "##") != label)
		{
			ImGui::SameLine();
			ImGui::TextUnformatted(visible_label(label));
		}

		const bool visible = begin_animated_area("##combo_area", open);
		if (visible)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, active_context()->style.rounding);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
			ImGui::PushStyleColor(ImGuiCol_ChildBg, to_imvec(active_context()->style.colors.panel));
			ImGui::PushStyleColor(ImGuiCol_Border, to_imvec(active_context()->style.colors.border));
			ImGui::BeginChild("##combo_panel", ImVec2(width, 160.0f), true);
			if (g_active_combo == id)
			{
				g_active_combo_field_min = pos;
				g_active_combo_field_max = add(pos, frame_size);
				g_active_combo_popup_min = ImGui::GetWindowPos();
				g_active_combo_popup_max = add(g_active_combo_popup_min, ImGui::GetWindowSize());
			}
			return true;
		}
		ImGui::PopID();
		return false;
	}

	void end_combo()
	{
		ImGui::EndChild();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(2);
		end_animated_area();
		ImGui::PopID();
	}

	bool combo(const char* label, int* current_item, const char* const items[], int item_count)
	{
		if (current_item == nullptr || items == nullptr || item_count <= 0)
		{
			return false;
		}
		const int index = std::max(0, Math::min(*current_item, item_count - 1));
		bool changed    = false;
		if (begin_combo(label, items[index]))
		{
			for (int i = 0; i < item_count; ++i)
			{
				if (selectable(items[i], *current_item == i))
				{
					*current_item = i;
					changed       = true;
					if (g_active_combo != 0)
					{
						g_open[g_active_combo] = false;
						g_active_combo         = 0;
					}
				}
			}
			end_combo();
		}
		return changed;
	}

	bool selectable(const char* label, bool selected, SelectableFlags flags, const Vec2& size)
	{
		tree_node_options options;
		options.selected = selected;
		options.leaf     = true;
		(void) flags;
		const bool clicked = tree_leaf(label, options);
		if (clicked && g_active_combo != 0)
		{
			g_open[g_active_combo] = false;
			g_active_combo         = 0;
		}
		return clicked;
	}

	bool radio_button(const char* label, bool active)
	{
		cleanup_states();
		ImGui::PushID(label);
		const ImGuiID id       = ImGui::GetID("radio");
		AnimState& anim        = state_for(id);
		const float diameter   = 20.0f;
		const ImVec2 text_size = ImGui::CalcTextSize(visible_label(label));
		const ImVec2 size(diameter + active_context()->style.spacing + text_size.x,
		                  std::max(active_context()->style.frame_height, diameter));
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("##radio", size);
		const bool clicked = ImGui::IsItemClicked();
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.active        = approach(anim.active, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.selected      = approach(anim.selected, active ? 1.0f : 0.0f, active_context()->style.animation_speed);

		ImDrawList* draw = ImGui::GetWindowDrawList();
		const ImVec2 center(pos.x + diameter * 0.5f, pos.y + size.y * 0.5f);
		draw->AddCircle(center, diameter * 0.5f,
		                col_u32(Math::lerp(active_context()->style.colors.border, active_context()->style.colors.accent,
		                                   anim.hover + anim.selected)),
		                24, 2.0f);
		draw->AddCircleFilled(center, diameter * 0.31f * apply_ease(anim.selected, ease::out_back),
		                      col_u32(active_context()->style.colors.accent, anim.selected));
		draw->AddText(ImVec2(pos.x + diameter + active_context()->style.spacing, pos.y + (size.y - text_size.y) * 0.5f),
		              col_u32(active_context()->style.colors.text), visible_label(label));
		ImGui::PopID();
		return clicked;
	}

	bool radio_button(const char* label, int* value, int button_value)
	{
		const bool clicked = radio_button(label, value != nullptr && *value == button_value);
		if (clicked && value != nullptr && *value != button_value)
		{
			*value = button_value;
			return true;
		}
		return false;
	}

	bool segmented_control(const char* label, int* current_item, const char* const items[], int item_count, const Vec2& size_arg)
	{
		if (current_item == nullptr || items == nullptr || item_count <= 0)
		{
			return false;
		}
		cleanup_states();
		ImGui::PushID(label);
		const float width     = size_arg.x > 0.0f ? size_arg.x : ImGui::GetContentRegionAvail().x;
		const float height    = size_arg.y > 0.0f ? size_arg.y : active_context()->style.frame_height;
		const float segment_w = width / static_cast<float>(item_count);
		const ImVec2 start    = ImGui::GetCursorScreenPos();
		ImDrawList* draw      = ImGui::GetWindowDrawList();
		draw->AddRectFilled(start, add(start, ImVec2(width, height)), col_u32(active_context()->style.colors.background),
		                    active_context()->style.rounding);
		draw->AddRect(start, add(start, ImVec2(width, height)), col_u32(active_context()->style.colors.border),
		              active_context()->style.rounding);

		bool changed = false;
		for (int i = 0; i < item_count; ++i)
		{
			ImGui::PushID(i);
			ImGui::SetCursorScreenPos(ImVec2(start.x + segment_w * i, start.y));
			ImGui::InvisibleButton("##segment", ImVec2(segment_w, height));
			const bool hovered = ImGui::IsItemHovered();
			const bool clicked = ImGui::IsItemClicked();
			AnimState& anim    = state_for(ImGui::GetID("segment_anim"));
			anim.hover         = approach(anim.hover, hovered ? 1.0f : 0.0f, active_context()->style.animation_speed);
			anim.selected = approach(anim.selected, *current_item == i ? 1.0f : 0.0f, active_context()->style.animation_speed);
			const ImVec2 min(start.x + segment_w * i, start.y);
			const ImVec2 max(start.x + segment_w * (i + 1), start.y + height);
			if (anim.hover > 0.01f)
			{
				draw->AddRectFilled(min, max, col_u32(active_context()->style.colors.background_hovered, anim.hover * 0.45f),
				                    i == 0 || i == item_count - 1 ? active_context()->style.rounding : 0.0f);
			}
			if (anim.selected > 0.01f)
			{
				draw->AddRectFilled(min, max, col_u32(active_context()->style.colors.accent, 0.28f * anim.selected),
				                    i == 0 || i == item_count - 1 ? active_context()->style.rounding : 0.0f);
				draw->AddRectFilled(ImVec2(min.x + 8.0f, max.y - 3.0f), ImVec2(max.x - 8.0f, max.y),
				                    col_u32(active_context()->style.colors.accent, anim.selected), 2.0f);
			}
			if (i > 0)
			{
				draw->AddLine(ImVec2(min.x, min.y + 6.0f), ImVec2(min.x, max.y - 6.0f),
				              col_u32(active_context()->style.colors.border));
			}
			const ImVec2 ts = ImGui::CalcTextSize(items[i]);
			draw->AddText(ImVec2(min.x + (segment_w - ts.x) * 0.5f, min.y + (height - ts.y) * 0.5f),
			              col_u32(active_context()->style.colors.text), items[i]);
			if (clicked && *current_item != i)
			{
				*current_item = i;
				changed       = true;
			}
			ImGui::PopID();
		}
		ImGui::SetCursorScreenPos(ImVec2(start.x, start.y + height));
		ImGui::Dummy(ImVec2(width, 0.0f));
		ImGui::PopID();
		return changed;
	}

	void progress_bar(float fraction, const Vec2& size_arg, const char* overlay)
	{
		cleanup_states();
		const ImGuiID id = ImGui::GetID("ui_progress_bar");
		AnimState& anim  = state_for(id);
		anim.value       = approach(anim.value, Math::clamp(fraction, 0.f, 1.f), active_context()->style.animation_speed);
		ImVec2 size      = to_imvec(size_arg);
		if (size.x < 0.0f)
		{
			size.x = ImGui::GetContentRegionAvail().x;
		}
		if (size.y <= 0.0f)
		{
			size.y = 10.0f;
		}
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::Dummy(size);
		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->AddRectFilled(pos, add(pos, size), col_u32(active_context()->style.colors.background_active), size.y * 0.5f);
		draw->AddRectFilled(pos, ImVec2(pos.x + size.x * anim.value, pos.y + size.y),
		                    col_u32(active_context()->style.colors.accent), size.y * 0.5f);
		if (overlay != nullptr)
		{
			const ImVec2 ts = ImGui::CalcTextSize(overlay);
			draw->AddText(ImVec2(pos.x + (size.x - ts.x) * 0.5f, pos.y + (size.y - ts.y) * 0.5f),
			              col_u32(active_context()->style.colors.text), overlay);
		}
	}

	void spinner(const char* id, float radius, float thickness, const Vec4& color)
	{
		ImGui::PushID(id);
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size(radius * 2.0f + thickness * 2.0f, radius * 2.0f + thickness * 2.0f);
		ImGui::Dummy(size);
		ImDrawList* draw    = ImGui::GetWindowDrawList();
		const ImVec2 center = add(pos, mul(size, 0.5f));
		const float start   = static_cast<float>(ImGui::GetTime() * 6.0);
		const ImU32 c       = col_u32(has_color(color) ? color : active_context()->style.colors.accent);
		for (int i = 0; i < 24; ++i)
		{
			const float a0    = start + (static_cast<float>(i) / 24.0f) * 6.2831853f;
			const float a1    = start + (static_cast<float>(i + 1) / 24.0f) * 6.2831853f;
			const float alpha = static_cast<float>(i + 1) / 24.0f;
			draw->AddLine(ImVec2(center.x + std::cos(a0) * radius, center.y + std::sin(a0) * radius),
			              ImVec2(center.x + std::cos(a1) * radius, center.y + std::sin(a1) * radius),
			              col_u32(has_color(color) ? color : active_context()->style.colors.accent, alpha), thickness);
			(void) c;
		}
		ImGui::PopID();
	}

	bool color_edit3(const char* label, Vec4* color, ColorEditFlags flags)
	{
		if (color == nullptr)
		{
			return false;
		}
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, active_context()->style.rounding);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, to_imvec(active_context()->style.colors.background));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, to_imvec(active_context()->style.colors.background_hovered));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, to_imvec(active_context()->style.colors.background_active));
		const bool changed = ImGui::ColorEdit3(label, &color->x, to_imgui_color_edit_flags(flags));
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		return changed;
	}

	bool color_edit4(const char* label, Vec4* color, ColorEditFlags flags)
	{
		if (color == nullptr)
		{
			return false;
		}
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, active_context()->style.rounding);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, to_imvec(active_context()->style.colors.background));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, to_imvec(active_context()->style.colors.background_hovered));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, to_imvec(active_context()->style.colors.background_active));
		const bool changed = ImGui::ColorEdit4(label, &color->x, to_imgui_color_edit_flags(flags));
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		return changed;
	}

	String keybind_to_string(const keybind& binding)
	{
		if (binding.key_code == ui::key::none)
		{
			return "None";
		}
		String result;
		if (binding.ctrl)
			result += "Ctrl+";
		if (binding.shift)
			result += "Shift+";
		if (binding.alt)
			result += "Alt+";
		if (binding.super)
			result += "Super+";
		result += ImGui::GetKeyName(to_imgui_key(binding.key_code));
		return result;
	}

	bool is_keybind_pressed(const keybind& binding, bool repeat)
	{
		return binding.key_code != ui::key::none && keybind_mods_match(binding) &&
		       ImGui::IsKeyPressed(to_imgui_key(binding.key_code), repeat);
	}

	bool keybind_input(const char* label, keybind* binding)
	{
		if (binding == nullptr)
		{
			return false;
		}

		cleanup_states();
		ImGui::PushID(label);
		const ImGuiID id        = ImGui::GetID("keybind");
		AnimState& anim         = state_for(id);
		const String display    = active_context()->keybind_capture == id ? "Press key..." : keybind_to_string(*binding);
		const ImVec2 label_size = ImGui::CalcTextSize(visible_label(label));
		const ImVec2 field_size(std::max(130.0f, ImGui::CalcItemWidth() * 0.55f), active_context()->style.frame_height);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(visible_label(label));
		ImGui::SameLine();
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("##keybind_button", field_size);
		const bool hovered = ImGui::IsItemHovered();
		if (ImGui::IsItemClicked())
		{
			active_context()->keybind_capture = id;
		}
		anim.hover       = approach(anim.hover, hovered ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.focus       = approach(anim.focus, active_context()->keybind_capture == id ? 1.0f : 0.0f,
		                            active_context()->style.animation_speed);
		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->AddRectFilled(pos, add(pos, field_size),
		                    col_u32(Math::lerp(active_context()->style.colors.background,
		                                       active_context()->style.colors.background_hovered, anim.hover)),
		                    active_context()->style.rounding);
		draw->AddRect(
		        pos, add(pos, field_size),
		        col_u32(Math::lerp(active_context()->style.colors.border, active_context()->style.colors.accent, anim.focus)),
		        active_context()->style.rounding, 0, active_context()->style.border_size);
		const ImVec2 ts = ImGui::CalcTextSize(display.c_str());
		draw->AddText(ImVec2(pos.x + active_context()->style.padding, pos.y + (field_size.y - ts.y) * 0.5f),
		              col_u32(active_context()->style.colors.text), display.c_str());
		(void) label_size;

		bool changed = false;

		if (active_context()->keybind_capture == id)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				active_context()->keybind_capture = 0;
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_Backspace) || ImGui::IsKeyPressed(ImGuiKey_Delete))
			{
				*binding                          = keybind{};
				active_context()->keybind_capture = 0;
				changed                           = true;
			}
			else
			{
				for (int key_i = static_cast<int>(ui::key::named_key_begin); key_i < static_cast<int>(ui::key::named_key_end);
				     ++key_i)
				{
					const ui::key key = Key(static_cast<Key::Enum>(key_i));
					if (!is_modifier_key(key) && ImGui::IsKeyPressed(to_imgui_key(key), false))
					{
						ImGuiIO& io                       = ImGui::GetIO();
						binding->key_code                 = key;
						binding->ctrl                     = io.KeyCtrl;
						binding->shift                    = io.KeyShift;
						binding->alt                      = io.KeyAlt;
						binding->super                    = io.KeySuper;
						active_context()->keybind_capture = 0;
						changed                           = true;
						break;
					}
				}
			}
		}
		ImGui::PopID();
		return changed;
	}

	bool collapsing_header(const char* label, const header_options& options)
	{
		ImGui::PushID(label);
		const ImGuiID id = ImGui::GetID("header_open");
		bool& stored     = g_open[id];
		static std::unordered_map<ImGuiID, bool> initialized;
		if (!initialized[id])
		{
			stored          = options.open != nullptr ? *options.open : options.default_open;
			initialized[id] = true;
		}
		bool open       = options.open != nullptr ? *options.open : stored;
		AnimState& anim = state_for(id);
		anim.open       = approach(anim.open, open ? 1.0f : 0.0f, active_context()->style.animation_speed);
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
		ImGui::PopID();
		return open;
	}

	bool section_header(const char* label, const header_options& options)
	{
		spacing(2.0f);
		return collapsing_header(label, options);
	}

	bool tree_node(const char* label, const tree_node_options& options)
	{
		ImGui::PushID(label);
		const ImGuiID id = ImGui::GetID("tree_open");
		bool& stored     = g_open[id];
		static std::unordered_map<ImGuiID, bool> initialized;
		if (!initialized[id])
		{
			stored          = options.open != nullptr ? *options.open : options.default_open;
			initialized[id] = true;
		}
		bool open          = options.leaf ? false : (options.open != nullptr ? *options.open : stored);
		AnimState& anim    = state_for(id);
		const float indent = g_tree_indent_stack.empty() ? 0.0f : g_tree_indent_stack.back();
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
		const float eased_open    = apply_ease(anim.open, ease::in_out_quad);
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
			g_tree_stack.push_back(context);
			active_context()->draw_alpha *= eased_open;
			g_tree_indent_stack.push_back(indent + 18.0f);
			return true;
		}
		ImGui::PopID();
		return false;
	}

	void tree_pop()
	{
		if (g_tree_stack.empty())
		{
			return;
		}

		tree_context context = g_tree_stack.back();
		g_tree_stack.pop_back();

		if (!g_tree_indent_stack.empty())
		{
			g_tree_indent_stack.pop_back();
		}

		ImVec2 content_end          = ImGui::GetCursorScreenPos();
		const float measured_height = std::max(0.0f, content_end.y - context.content_start.y);
		AnimState& anim             = state_for(context.id);
		if (context.logical_open && measured_height > 0.0f)
		{
			anim.extra = measured_height;
		}

		const float eased_open     = apply_ease(anim.open, ease::in_out_quad);
		const float cached_height  = std::max(anim.extra, measured_height);
		const float visible_height = cached_height * eased_open;

		active_context()->draw_alpha = context.previous_draw_alpha;
		ImGui::PopStyleVar();
		ImGui::PopClipRect();

		ImGui::SetCursorScreenPos(ImVec2(context.content_start.x, context.content_start.y + visible_height));
		ImGui::PopID();
	}

	bool tree_leaf(const char* label, const tree_node_options& options)
	{
		tree_node_options copy = options;
		copy.leaf              = true;
		const float indent     = g_tree_indent_stack.empty() ? 0.0f : g_tree_indent_stack.back();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);
		return animated_row(label, copy.icon, copy.badge, copy.selected, false,
		                    ImVec2(ImGui::GetContentRegionAvail().x, active_context()->style.frame_height), copy.accent, false,
		                    0.0f);
	}

	bool tree_item(const char* label, const tree_node_options& options)
	{
		return tree_leaf(label, options);
	}

	bool selectable_tree_item(const char* label, bool selected, const tree_node_options& options)
	{
		tree_node_options copy = options;
		copy.selected          = selected;
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
		button_options options;
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
			g_pending_modals.emplace_back(name);
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
			g_pending_popups.emplace_back(id);
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
			++g_menu_bar_style_depth;
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
		if (g_menu_bar_style_depth > 0)
		{
			--g_menu_bar_style_depth;
			pop_menu_bar_colors();
		}
	}

	bool begin_menu(const char* label, bool enabled)
	{
		const ImGuiID id  = ImGui::GetID(label);
		AnimState& anim   = state_for(id);
		const float alpha = std::max(0.001f, apply_ease(anim.open, ease::in_out_quad));
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
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * apply_ease(anim.open, ease::in_out_quad));
			++g_menu_alpha_depth;
			++g_menu_popup_style_depth;
		}
		else
		{
			pop_menu_popup_colors();
		}
		return open;
	}

	void end_menu()
	{
		if (g_menu_alpha_depth > 0)
		{
			--g_menu_alpha_depth;
			ImGui::PopStyleVar();
		}
		ImGui::EndMenu();
		if (g_menu_popup_style_depth > 0)
		{
			--g_menu_popup_style_depth;
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
		g_notifications.push_back(std::move(n));
	}

	ConfirmResult confirmation(const char* title, const char* message, const char* confirm_text, const char* cancel_text,
	                           bool danger)
	{
		confirm_result result = confirm_result::none;
		bool open             = true;
		if (begin_modal(title, &open))
		{
			text("%s", message != nullptr ? message : "");
			spacing();
			if ((danger ? danger_button(confirm_text) : button(confirm_text)))
			{
				result = confirm_result::confirmed;
				ImGui::CloseCurrentPopup();
			}
			same_line();
			if (ghost_button(cancel_text))
			{
				result = confirm_result::cancelled;
				ImGui::CloseCurrentPopup();
			}
			end_modal();
		}
		if (!open && result == confirm_result::none)
		{
			result = confirm_result::cancelled;
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
		property_row(label, [&] { changed = slider_float("##value", value, min, max, format); }, label_width);
		return changed;
	}

	bool property_int(const char* label, int* value, int min, int max, const char* format, float label_width)
	{
		bool changed = false;
		property_row(label, [&] { changed = slider_int("##value", value, min, max, format); }, label_width);
		return changed;
	}

	bool property_text(const char* label, char* buffer, size_t buffer_size, float label_width)
	{
		bool changed = false;
		property_row(label, [&] { changed = input_text("##value", buffer, buffer_size); }, label_width);
		return changed;
	}

	bool property_color(const char* label, Vec4* color, bool alpha, float label_width)
	{
		bool changed = false;
		property_row(
		        label, [&] { changed = alpha ? color_edit4("##value", color) : color_edit3("##value", color); }, label_width);
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
		panel_options options;
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
			++g_table_style_depth;
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
		if (g_table_style_depth > 0)
		{
			--g_table_style_depth;
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


	///////////////////////////////// API VIEWPORT EXAMPLE /////////////////////////////////
}// namespace Trinex::UI


namespace Trinex
{
	class ENGINE_EXPORT UITest : public ViewportClient
	{
		trinex_class(UITest, ViewportClient);
		UI::Context* m_ctx;

		bool enabled               = true;
		bool fullscreen            = false;
		bool show_grid             = true;
		bool visible               = true;
		bool modal_open            = true;
		bool advanced_visible      = true;
		bool menu_grid             = true;
		float opacity              = 0.82f;
		float bloom                = 0.35f;
		float exposure             = 1.25f;
		float progress             = 0.62f;
		float left_size            = 260.0f;
		float right_size           = 1020.0f;
		int quality                = 2;
		int radio_mode             = 0;
		int segmented_mode         = 1;
		int selected_tab           = 0;
		int selected_sidebar       = 0;
		int selected_entity        = 0;
		int selected_list_item     = 0;
		int combo_index            = 1;
		char name_buffer[128]      = "Player";
		char search_buffer[128]    = "";
		char filter_buffer[128]    = "";
		char dropped_payload[128]  = "Drop an item here";
		UI::Vec4 tint_color        = UI::Vec4(0.28f, 0.62f, 0.95f, 1.0f);
		const char* combo_items[4] = {"Low", "Medium", "High", "Ultra"};
		const char* mode_items[3]  = {"Move", "Rotate", "Scale"};
		const char* list_items[7]  = {"Camera",       "Player",         "Light",          "Environment",
		                              "Post Process", "Audio Listener", "Navigation Mesh"};

	public:
		UITest& on_bind_viewport(class RenderViewport* viewport) override
		{
			Super::on_bind_viewport(viewport);
			m_ctx = UI::create_context(viewport->window());
			return *this;
		}

		UITest& on_unbind_viewport(class RenderViewport* viewport) override
		{
			Super::on_bind_viewport(viewport);
			UI::destroy_context(m_ctx);
			return *this;
		}

		UITest& update(class RenderViewport* viewport, float dt) override
		{
			Super::update(viewport, dt);

			UI::begin_frame(m_ctx);
			{
				UI::style accent_style     = UI::get_style();
				accent_style.colors.accent = UI::Vec4(0.95f, 0.54f, 0.25f, 1.0f);

				if (UI::begin_window("Animated UI Framework Showcase", nullptr, ImGuiWindowFlags_MenuBar))
				{
					if (UI::begin_menu_bar())
					{
						if (UI::begin_menu("File"))
						{
							if (UI::menu_item("Confirm reset", nullptr))
							{
								UI::open_modal("High-level confirm");
							}
							UI::end_menu();
						}
						if (UI::begin_menu("View"))
						{
							UI::menu_item("Show grid", nullptr, &menu_grid);
							UI::menu_item("Disabled item", "Ctrl+D", false, false);

							if (UI::begin_menu("View Mode"))
							{
								static bool flags[5];
								UI::menu_item("Base Color", nullptr, &flags[0]);
								UI::menu_item("Normal", nullptr, &flags[1]);
								UI::menu_item("Roughness", nullptr, &flags[2]);
								UI::menu_item("Metalic", nullptr, &flags[3]);
								UI::menu_item("AO", nullptr, &flags[4]);
								UI::end_menu();
							}

							UI::end_menu();
						}
						UI::end_menu_bar();
					}

					const UI::confirm_result confirm = UI::confirmation(
					        "High-level confirm", "Reset demo values using confirmation_modal()?", "Reset", "Cancel", true);
					if (confirm == UI::confirm_result::confirmed)
					{
						opacity  = 0.82f;
						bloom    = 0.35f;
						exposure = 1.25f;
						progress = 0.62f;
						UI::notification("High-level confirmation accepted.", {UI::notification_kind::warning, 3.0f, "Confirm"});
					}

					UI::text("Immediate-mode UI framework layer");
					UI::text_muted("A compact showcase of the public UI::* API.");
					UI::same_line();
					UI::help_marker(
					        "Every section uses custom animated widgets while preserving Dear ImGui's immediate-mode usage.");
					UI::separator();

					if (UI::begin_toolbar("top_toolbar"))
					{
						if (UI::button("Save"))
						{
							UI::notification("Scene saved successfully.", {UI::notification_kind::success, 3.0f, "Save"});
						}
						UI::same_line();
						if (UI::icon_button("+", "Create"))
						{
							UI::notification("Created a new object.", {UI::notification_kind::info, 2.5f, "Create"});
						}
						UI::same_line();
						if (UI::ghost_button("Open popup"))
						{
							UI::open_popup("demo_popup");
						}
						UI::same_line();
						if (UI::danger_button("Open modal"))
						{
							modal_open = true;
							UI::open_modal("Confirm reset");
						}
						UI::end_toolbar();
					}

					if (UI::begin_popup("demo_popup"))
					{
						UI::text("Popup menu");
						UI::separator();
						if (UI::selectable("Duplicate"))
						{
							UI::notification("Duplicate selected.");
						}
						if (UI::selectable("Rename"))
						{
							UI::notification("Rename selected.");
						}
						if (UI::selectable("Delete"))
						{
							UI::notification("Delete selected.", {UI::notification_kind::warning, 3.0f, "Popup"});
						}
						UI::end_popup();
					}

					if (UI::begin_modal("Confirm reset", &modal_open))
					{
						UI::text("Reset all demo values?");
						UI::text_muted("This demonstrates begin_modal(), open_modal(), and end_modal().");
						UI::spacing();
						if (UI::danger_button("Reset"))
						{
							enabled    = true;
							fullscreen = false;
							show_grid  = true;
							visible    = true;
							opacity    = 0.82f;
							bloom      = 0.35f;
							progress   = 0.62f;
							quality    = 2;
							UI::notification("Values were reset.", {UI::notification_kind::warning, 3.0f, "Reset"});
							modal_open = false;
						}
						UI::same_line();
						if (UI::ghost_button("Cancel"))
						{
							modal_open = false;
						}
						UI::end_modal();
					}

					UI::panel_options sidebar_panel;
					sidebar_panel.size = UI::Vec2(left_size, 0.0f);
					if (UI::begin_child_panel("sidebar_panel", sidebar_panel.size, sidebar_panel))
					{
						UI::text_muted("Navigation");
						if (UI::sidebar_item("Controls", selected_sidebar == 0, "01", "Core"))
						{
							selected_sidebar = 0;
						}
						if (UI::sidebar_item("Hierarchy", selected_sidebar == 1, "02", "Tree"))
						{
							selected_sidebar = 1;
						}
						if (UI::sidebar_item("Layout", selected_sidebar == 2, "03", "Panels"))
						{
							selected_sidebar = 2;
						}
						if (UI::nav_item("Utilities", selected_sidebar == 3, "04"))
						{
							selected_sidebar = 3;
						}
						if (UI::nav_item("Data Views", selected_sidebar == 4, "05"))
						{
							selected_sidebar = 4;
						}

						UI::separator();
						UI::breadcrumb("Root");
						UI::breadcrumb(selected_sidebar == 0   ? "Controls"
						               : selected_sidebar == 1 ? "Hierarchy"
						               : selected_sidebar == 2 ? "Layout"
						               : selected_sidebar == 3 ? "Utilities"
						                                       : "Data Views",
						               true);

						UI::spacing();
						UI::badge("LIVE", UI::get_style().colors.success);
						UI::same_line();
						UI::pill("BETA", UI::get_style().colors.warning);
						UI::same_line();
						UI::status_dot(enabled ? UI::get_style().colors.success : UI::get_style().colors.error);
					}
					UI::end_child_panel();

					UI::same_line();
					UI::splitter("main_splitter", &left_size, &right_size, 180.0f, 360.0f);
					UI::same_line();

					UI::panel_options content_panel;
					content_panel.size = UI::Vec2(right_size, 0.0f);
					if (UI::begin_child_panel("content_panel", content_panel.size, content_panel))
					{
						if (UI::begin_tab_bar("showcase_tabs"))
						{
							if (UI::tab("Controls", selected_tab == 0))
							{
								selected_tab = 0;
							}
							UI::same_line();
							if (UI::tab("Tree", selected_tab == 1))
							{
								selected_tab = 1;
							}
							UI::same_line();
							if (UI::tab("Text/Layout", selected_tab == 2))
							{
								selected_tab = 2;
							}
							UI::same_line();
							if (UI::tab("Style", selected_tab == 3))
							{
								selected_tab = 3;
							}
							UI::same_line();
							if (UI::tab("Extended", selected_tab == 4))
							{
								selected_tab = 4;
							}
							UI::same_line();
							if (UI::tab("Data", selected_tab == 5))
							{
								selected_tab = 5;
							}
							UI::end_tab_bar();
						}

						UI::spacing();

						if (selected_tab == 0)
						{
							UI::text("Controls");
							UI::text_muted("Buttons, toggles, inputs, combo boxes, sliders, progress, and spinner.");

							if (UI::button("Primary button"))
							{
								UI::notification("Primary button clicked.");
							}
							UI::same_line();
							if (UI::icon_button("*", "Icon button"))
							{
								UI::notification("Icon button clicked.");
							}
							UI::same_line();
							UI::small_button("Small");

							UI::spacing();
							UI::ghost_button("Ghost button");
							UI::same_line();
							UI::danger_button("Danger button");
							UI::same_line();
							UI::image_button("##test", DefaultResources::Textures::noise128x128->rhi_texture(), {128, 128});
							UI::same_line();
							UI::image(DefaultResources::Textures::noise128x128->rhi_texture(), {128, 128});

							UI::separator();
							UI::checkbox("Show grid", &show_grid);
							UI::toggle("Enabled", &enabled);
							UI::toggle("Visible", &visible);

							UI::slider_float("Opacity", &opacity, 0.0f, 1.0f);
							UI::slider_float("Bloom", &bloom, 0.0f, 1.0f);
							UI::slider_int("Quality", &quality, 0, 3);
							UI::slider_float("Progress", &progress, 0.0f, 1.0f);

							UI::input_text("Name", name_buffer, sizeof(name_buffer));
							UI::search_input("Search", search_buffer, sizeof(search_buffer));

							UI::combo("Preset helper", &combo_index, combo_items, 4);

							if (UI::begin_combo("Preset", combo_items[combo_index]))
							{
								for (int i = 0; i < 4; ++i)
								{
									if (UI::selectable(combo_items[i], combo_index == i))
									{
										combo_index = i;
									}
								}
								UI::end_combo();
							}

							UI::progress_bar(progress, UI::Vec2(-1.0f, 14.0f), "Animated progress");
							UI::spacing();
							UI::spinner("main_spinner", 10.0f, 2.5f);
						}
						else if (selected_tab == 1)
						{
							UI::text("Hierarchy");
							UI::text_muted("Animated tree_node() content expands, fades, clips, and collapses smoothly.");

							UI::tree_node_options scene_node;
							scene_node.icon         = "S";
							scene_node.badge        = "3";
							scene_node.default_open = true;
							if (UI::tree_node("Scene", scene_node))
							{
								UI::tree_node_options camera_node;
								camera_node.icon     = "C";
								camera_node.selected = selected_entity == 0;
								if (UI::selectable_tree_item("Camera", selected_entity == 0, camera_node))
								{
									selected_entity = 0;
								}

								UI::tree_node_options player_node;
								player_node.icon         = "P";
								player_node.badge        = "Open";
								player_node.selected     = selected_entity == 1;
								player_node.default_open = true;
								if (UI::tree_node("Player", player_node))
								{
									UI::tree_node_options transform_leaf;
									transform_leaf.icon = "T";
									if (UI::tree_leaf("Transform", transform_leaf))
									{
										selected_entity = 1;
									}

									UI::tree_node_options renderer_leaf;
									renderer_leaf.icon  = "R";
									renderer_leaf.badge = "Mesh";
									if (UI::tree_item("Renderer", renderer_leaf))
									{
										selected_entity = 1;
									}

									UI::tree_node_options inventory_node;
									inventory_node.icon         = "I";
									inventory_node.default_open = true;
									if (UI::tree_node("Inventory", inventory_node))
									{
										UI::tree_leaf("Weapon");
										UI::tree_leaf("Shield");
										UI::tree_leaf("Potion");
										UI::tree_pop();
									}

									UI::tree_pop();
								}

								UI::tree_node_options light_node;
								light_node.icon     = "L";
								light_node.selected = selected_entity == 2;
								if (UI::selectable_tree_item("Light", selected_entity == 2, light_node))
								{
									selected_entity = 2;
								}

								UI::tree_pop();
							}

							UI::separator();
							UI::section_header("Inspector");
							UI::property_row("Selected", [&] {
								UI::text(selected_entity == 0 ? "Camera" : selected_entity == 1 ? "Player" : "Light");
							});
							UI::property_row("Visible", [&] { UI::toggle("##inspector_visible", &visible); });
							UI::property_row("Opacity", [&] { UI::slider_float("##inspector_opacity", &opacity, 0.0f, 1.0f); });
							UI::property_bool("Grid", &show_grid);
							UI::property_float("Exposure", &exposure, 0.0f, 4.0f);
							UI::property_int("Quality", &quality, 0, 3);
							UI::property_text("Name", name_buffer, sizeof(name_buffer));
							UI::property_color("Tint", &tint_color);
							UI::key_value_row("Renderer", combo_items[combo_index]);
						}
						else if (selected_tab == 2)
						{
							UI::text("Text and layout");
							UI::text_muted("Panels, child panels, group panels, rows, labels, and tooltips.");
							UI::text_colored(UI::get_style().colors.accent, "Accent text");
							UI::label("Current name", name_buffer);
							UI::tooltip("This tooltip is shown explicitly from UI::tooltip().");
							UI::button("Hover for delayed tooltip");
							UI::tooltip_delayed("This appears after a short delay.");
							UI::same_line();
							UI::button("Hover for instant tooltip");
							UI::tooltip_if_hovered("This appears immediately.");
							UI::same_line();
							UI::help_tooltip("help_tooltip() is a compact wrapper around the marker pattern.");

							UI::separator();

							UI::panel_options nested_panel;
							nested_panel.size             = UI::Vec2(0.0f, 110.0f);
							nested_panel.background_color = UI::Vec4(0.08f, 0.13f, 0.12f, 1.0f);
							if (UI::begin_panel("nested_panel", nested_panel))
							{
								UI::text("begin_panel() / end_panel()");
								UI::text_muted("This panel uses a custom background color.");
								UI::button("Panel button");
							}
							UI::end_panel();

							UI::spacing();

							if (UI::begin_group_panel("Group panel", UI::Vec2(0.0f, 120.0f)))
							{
								UI::text("begin_group_panel() / end_group_panel()");
								UI::checkbox("Grouped checkbox", &show_grid);
								UI::toggle("Grouped toggle", &enabled);
							}
							UI::end_group_panel();

							UI::spacing();
							UI::key_value_row("Animation speed", "12.0");
						}
						else if (selected_tab == 3)
						{
							UI::text("Style and settings");
							UI::text_muted("Style access, pushed temporary style, headers, and notification variants.");

							if (UI::collapsing_header("Window", [] {
								    UI::header_options options;
								    options.right_text   = "State";
								    options.default_open = true;
								    return options;
							    }()))
							{
								UI::toggle("Fullscreen", &fullscreen);
								UI::toggle("VSync / Enabled", &enabled);
							}

							UI::header_options rendering_header;
							rendering_header.icon         = "R";
							rendering_header.right_text   = combo_items[combo_index];
							rendering_header.default_open = true;
							if (UI::section_header("Rendering", rendering_header))
							{
								UI::slider_int("Quality preset", &quality, 0, 3);
								UI::slider_float("Bloom intensity", &bloom, 0.0f, 1.0f);
							}

							UI::separator();
							UI::text("Temporary pushed style");
							UI::push_style(accent_style);
							if (UI::button("Orange accent via push_style()"))
							{
								UI::notification("Temporary style button clicked.", {UI::notification_kind::info, 3.0f, "Style"});
							}
							UI::pop_style();

							UI::spacing();
							if (UI::button("Info notification"))
							{
								UI::notification_options options;
								options.kind         = UI::notification_kind::info;
								options.duration     = 5.0f;
								options.title        = "Info";
								options.action_label = "Action";
								options.action       = [] {
                                    UI::notification("Notification action clicked.",
									                       {UI::notification_kind::success, 2.0f, "Action"});
								};
								UI::notification("Information message with action.", options);
							}
							UI::same_line();
							if (UI::button("Success notification"))
							{
								UI::notification("Success message.", {UI::notification_kind::success, 3.0f, "Success"});
							}
							UI::same_line();
							if (UI::button("Warning notification"))
							{
								UI::notification("Warning message.", {UI::notification_kind::warning, 3.0f, "Warning"});
							}
							UI::same_line();
							if (UI::button("Error notification"))
							{
								UI::notification("Error message.", {UI::notification_kind::error, 3.0f, "Error"});
							}

							char helper_text[128];
							std::snprintf(helper_text, sizeof(helper_text), "lerp(0, 10, 0.35) = %.2f, ease(0.5) = %.2f",
							              Math::lerp(0.0f, 10.0f, 0.35f), UI::apply_ease(0.5f, UI::ease::in_out_quad));
							UI::text_muted("%s", helper_text);
						}
						else if (selected_tab == 4)
						{
							UI::text("Extended controls");
							UI::text_muted("Disabled scopes, animated areas, radio controls, segmented controls, colors, "
							               "keybinds, RAII, "
							               "and states.");

							UI::toggle("Enable disabled-scope contents", &enabled);
							UI::begin_disabled(!enabled);
							UI::slider_float("Disabled-scope opacity", &opacity, 0.0f, 1.0f);
							UI::button("Disabled-scope button");
							UI::end_disabled();

							{
								UI::DisabledScope scope(!enabled);
								UI::ghost_button("RAII disabled_scope");
							}

							UI::separator();
							UI::toggle("Show advanced animated_area", &advanced_visible);
							if (UI::begin_animated_area("advanced_area", advanced_visible))
							{
								UI::slider_float("Exposure", &exposure, 0.0f, 4.0f);
								UI::slider_float("Bloom advanced", &bloom, 0.0f, 1.0f);
								UI::end_animated_area();
							}

							UI::animated_area("callback_area", advanced_visible, [&] {
								UI::text_muted("Callback animated_area() content.");
								UI::progress_bar(bloom, UI::Vec2(-1.0f, 10.0f));
							});

							UI::separator();
							UI::radio_button("Radio Move", &radio_mode, 0);
							UI::same_line();
							UI::radio_button("Radio Rotate", &radio_mode, 1);
							UI::same_line();
							UI::radio_button("Radio Scale", &radio_mode, 2);
							UI::segmented_control("Transform mode", &segmented_mode, mode_items, 3);

							UI::color_edit3("Tint RGB", &tint_color);
							UI::color_edit4("Tint RGBA", &tint_color);

							UI::separator();
							{
								UI::StyleScope scope(accent_style);
								UI::button("RAII style_scope");
							}
							{
								UI::IdScope scope("custom_id_scope");
								const float animated = UI::animate_float(UI::id("custom_float_anim"), progress);
								UI::progress_bar(animated, UI::Vec2(-1.0f, 10.0f), "animate_float()");
							}
							if (UI::button("Reset custom animation"))
							{
								UI::reset_animation(UI::id("custom_float_anim"));
							}
							UI::same_line();
							if (UI::button("Clear all animations"))
							{
								UI::clear_animations();
							}

							UI::separator();
							UI::empty_state({"-", "Empty state", "Use this in panels with no content."});
							UI::loading_state("Loading state");
							UI::error_state("Something went wrong while loading data.");
						}
						else
						{
							UI::text("Data views");
							UI::text_muted("Tables, list boxes, filtered lists, drag/drop, and scroll helpers.");

							if (UI::begin_table("entity_table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
							{
								UI::table_column("Name");
								UI::table_column("Type");
								UI::table_column("Visible");
								UI::table_headers();
								for (int i = 0; i < 4; ++i)
								{
									UI::table_next_row();
									UI::table_next_column();
									UI::text("%s", list_items[i]);
									UI::table_next_column();
									UI::text_muted("%s", i == 0 ? "Camera" : i == 1 ? "Entity" : i == 2 ? "Light" : "World");
									UI::table_next_column();
									UI::status_dot(i % 2 == 0 ? UI::get_style().colors.success : UI::get_style().colors.warning);
								}
								UI::end_table();
							}

							UI::separator();
							UI::search_input("Filter list", filter_buffer, sizeof(filter_buffer));
							if (UI::begin_list_box("Entities", UI::Vec2(0.0f, 150.0f)))
							{
								for (int i = 0; i < 7; ++i)
								{
									if (UI::list_item(list_items[i], selected_list_item == i, "E",
									                  i == selected_list_item ? "Selected" : nullptr))
									{
										selected_list_item = i;
									}
									if (UI::begin_drag_source())
									{
										UI::drag_payload_text("ENTITY_NAME", list_items[i]);
										UI::text("Dragging %s", list_items[i]);
										UI::end_drag_source();
									}
								}
								UI::end_list_box();
							}
							UI::filtered_list("filtered_entities", filter_buffer, list_items, 7, &selected_list_item);

							UI::button(dropped_payload, {.size = UI::Vec2(-1.0f, 36.0f)});
							if (UI::begin_drop_target())
							{
								// if (const ImGuiPayload* payload = UI::accept_drop_payload("ENTITY_NAME"))
								// {
								// 	std::snprintf(dropped_payload, sizeof(dropped_payload), "Dropped: %s",
								// 	              static_cast<const char*>(payload->Data));
								// }
								UI::end_drop_target();
							}

							UI::separator();
							if (UI::begin_scroll_area("scroll_area", UI::Vec2(0.0f, 120.0f), true))
							{
								for (int i = 0; i < 20; ++i)
								{
									UI::text("Scrollable row %d", i);
								}
								if (UI::button("Scroll to top"))
								{
									UI::scroll_to_top();
								}
								UI::same_line();
								if (UI::button("Scroll to bottom"))
								{
									UI::scroll_to_bottom();
								}
							}
							UI::end_scroll_area();
							UI::text_muted("Last submitted item visible: %s", UI::is_item_visible() ? "yes" : "no");
						}

						UI::end_child_panel();
					}


					UI::end_window();
				}
			}
			UI::end_frame();

			return *this;
		}
	};

	trinex_implement_class(UITest, 0) {}
}// namespace Trinex
