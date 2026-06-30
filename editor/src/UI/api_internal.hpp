#pragma once
#include <Core/editor_config.hpp>
#include <Core/etl/stack.hpp>
#include <Core/etl/vector.hpp>
#include <Core/file_manager.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/types/path.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_viewport.hpp>
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
#include <imgui_internal.h>
#include <imgui_stacklayout.h>
#include <limits>
#include <unordered_map>

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

	namespace
	{
		using tree_context = TreeContext;
		using area_context = AreaContext;

		static inline Context* active_context()
		{
			trinex_assert(g_context);
			return g_context;
		}

		static constexpr usize font_size_index(ui::FontSize size)
		{
			return static_cast<usize>(size);
		}

		static constexpr usize font_family_index(ui::FontFamily family)
		{
			return static_cast<usize>(family);
		}

		static constexpr f32 font_size_value(ui::FontSize size)
		{
			switch (size)
			{
				case ui::FontSize::Small: return Settings::Editor::small_font_size;
				case ui::FontSize::Large: return Settings::Editor::large_font_size;
				case ui::FontSize::Normal:
				default: return Settings::Editor::normal_font_size;
			}
		}

		static ui::FontSize closest_font_size(f32 pixel_size)
		{
			ui::FontSize result = ui::FontSize::Normal;
			f32 best_delta      = std::abs(pixel_size - font_size_value(result));

			for (usize i = 0; i < Context::font_size_count; ++i)
			{
				const ui::FontSize candidate = static_cast<ui::FontSize>(i);
				const f32 delta              = std::abs(pixel_size - font_size_value(candidate));

				if (delta < best_delta)
				{
					best_delta = delta;
					result     = candidate;
				}
			}

			return result;
		}

		static Buffer load_font_data(const Path& path)
		{
			FileReader reader(path);
			trinex_verify(reader.is_open());
			return reader.read_buffer();
		}

		static ImFont* register_font(const Buffer& buffer, const ImFontConfig& config, const ImWchar* range, f32 size)
		{
			auto& io     = ImGui::GetIO();
			void* memory = ImGui::MemAlloc(buffer.size());
			memcpy(memory, buffer.data(), buffer.size());
			return io.Fonts->AddFontFromMemoryTTF(memory, static_cast<int>(buffer.size()), size, &config, range);
		}

		static void initialize_fonts(Context* ctx)
		{
			auto& io = ImGui::GetIO();

			const ImWchar icons_ranges[] = {
			        0x0020,      0x00FF,// ASCII + Latin
			        0x2000,      0x206F,// punctuation
			        0x2190,      0x21FF,// arrows
			        0x2700,      0x27BF,// dingbats
			        ICON_MIN_LC, ICON_MAX_LC, 0,
			};

			const Buffer text_font_data  = load_font_data(Settings::Editor::font_path);
			const Buffer icons_font_data = load_font_data("[content]:/TrinexEditor/fonts/Lucide/lucide.ttf");
			const ImWchar* text_ranges   = io.Fonts->GetGlyphRangesCyrillic();

			for (usize i = 0; i < Context::font_size_count; ++i)
			{
				const ui::FontSize size = static_cast<ui::FontSize>(i);
				const f32 pixel_size    = font_size_value(size);

				ImFontConfig text_cfg;
				text_cfg.FontDataOwnedByAtlas = true;
				ctx->fonts[font_family_index(ui::FontFamily::Text)][i] =
				        register_font(text_font_data, text_cfg, text_ranges, pixel_size);

				ImFontConfig icons_cfg;
				icons_cfg.FontDataOwnedByAtlas = true;
				icons_cfg.PixelSnapH           = true;
				ctx->fonts[font_family_index(ui::FontFamily::Icons)][i] =
				        register_font(icons_font_data, icons_cfg, icons_ranges, pixel_size);

				ImFontConfig default_cfg;
				default_cfg.FontDataOwnedByAtlas = true;
				ImFont* merged_font              = register_font(text_font_data, default_cfg, text_ranges, pixel_size);
				ctx->fonts[font_family_index(ui::FontFamily::Default)][i] = merged_font;

				ImFontConfig merged_icons_cfg;
				merged_icons_cfg.FontDataOwnedByAtlas = true;
				merged_icons_cfg.MergeMode            = true;
				merged_icons_cfg.DstFont              = merged_font;
				merged_icons_cfg.PixelSnapH           = true;
				register_font(icons_font_data, merged_icons_cfg, icons_ranges, pixel_size);
			}
		}

		static ImFont* resolve_font(Context* ctx, ui::FontFamily family, ui::FontSize size)
		{
			return ctx->fonts[font_family_index(family)][font_size_index(size)];
		}

		float dt()
		{
			return std::max(0.0f, ImGui::GetIO().DeltaTime);
		}

		void* memory_copy(void* memory, usize size)
		{
			if (memory == nullptr || size == 0)
			{
				return memory;
			}

			void* dst = StackByteAllocator::allocate(size);
			memcpy(dst, memory, size);
			return dst;
		}

		template<typename T>
		void destroy_list(T* list)
		{
			while (list)
			{
				T* next = list->next;
				trx_delete list;
				list = next;
			}
		}

		void add_paint_callback(ImDrawList* list, ImGuiViewport* vp, Vec2 pos, Vec2 size, PaintFunction function, void* userdata,
		                        usize userdata_size)
		{
			if (list == nullptr || vp == nullptr || function == nullptr)
			{
				return;
			}

			struct ViewportArgs {
				Trinex::Vector2f16 pos;
				Trinex::Vector2f16 size;
			} viewport_args;

			const Vec2 viewport_pos(vp->Pos.x, vp->Pos.y);
			const Vec2 viewport_size(vp->Size.x, vp->Size.y);
			viewport_args.pos  = (pos - viewport_pos) / viewport_size;
			viewport_args.size = size / viewport_size;

			ImDrawCallback viewport_setup = [](const ImDrawList*, const ImDrawCmd* cmd) {
				ViewportArgs* args = reinterpret_cast<ViewportArgs*>(cmd->UserCallbackData);

				auto ctx = Trinex::UI::Backend::rhi();
				ctx->viewport(Trinex::RHIRegion(args->size, args->pos));
				ctx->scissor(Trinex::RHIRegion(args->size, args->pos));
			};

			list->AddCallback(viewport_setup, &viewport_args, sizeof(viewport_args));

			if (userdata == nullptr)
			{
				ImDrawCallback callback = [](const ImDrawList*, const ImDrawCmd* cmd) {
					PaintFunction* function = static_cast<PaintFunction*>(cmd->UserCallbackData);
					(*function)(nullptr);
				};

				list->AddCallback(callback, &function, sizeof(function));
			}
			else
			{
				struct CallbackArgs {
					PaintFunction function;
					void* userdata;
				};

				CallbackArgs callback_args = {.function = function, .userdata = memory_copy(userdata, userdata_size)};

				ImDrawCallback callback = [](const ImDrawList*, const ImDrawCmd* cmd) {
					CallbackArgs* args = static_cast<CallbackArgs*>(cmd->UserCallbackData);
					args->function(args->userdata);
				};

				list->AddCallback(callback, &callback_args, sizeof(callback_args));
			}

			list->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
		}

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
			const float delta_time = std::max(0.0f, dt());
			const float s          = std::max(0.0f, speed);

			const float k    = -std::expm1(-s * delta_time);
			const float next = current + (target - current) * k;

			if (std::abs(target - next) <= epsilon)
			{
				return target;
			}

			return next;
		}

		inline float approach(float current, float target)
		{
			return approach(current, target, active_context()->style.animation_speed);
		}

		AnimState& state_for(ImGuiID id)
		{
			AnimState& state = active_context()->anim[id];

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

			auto& anim = active_context()->anim;
			for (auto it = anim.begin(); it != anim.end();)
			{
				if (frame - it->second.last_frame > 1200)
				{
					it = anim.erase(it);
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

		PersistentWindow* find_window(Context* context, Widget* widget)
		{
			if (context == nullptr || widget == nullptr)
			{
				return nullptr;
			}

			PersistentWindow* window = context->window_list;

			while (window)
			{
				if (window->widget == widget)
				{
					return window;
				}

				window = window->next;
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

		ImGuiDockNodeFlags to_imgui_dock_window_flags(DockWindowFlags value)
		{
			ImGuiDockNodeFlags flags = 0;
			if ((value & DockWindowFlags::NoSplit) != DockWindowFlags::Undefined)
				flags |= ImGuiDockNodeFlags_NoSplit;
			if ((value & DockWindowFlags::NoResize) != DockWindowFlags::Undefined)
				flags |= ImGuiDockNodeFlags_NoResize;
			if ((value & DockWindowFlags::AutoHideTabBar) != DockWindowFlags::Undefined)
				flags |= ImGuiDockNodeFlags_AutoHideTabBar;
			if ((value & DockWindowFlags::NoUndocking) != DockWindowFlags::Undefined)
				flags |= ImGuiDockNodeFlags_NoUndocking;
			if ((value & DockWindowFlags::NoTabBar) != DockWindowFlags::Undefined)
				flags |= ImGuiDockNodeFlags_NoTabBar;
			if ((value & DockWindowFlags::HiddenTabBar) != DockWindowFlags::Undefined)
				flags |= ImGuiDockNodeFlags_HiddenTabBar;
			if ((value & DockWindowFlags::NoWindowMenuButton) != DockWindowFlags::Undefined)
				flags |= ImGuiDockNodeFlags_NoWindowMenuButton;
			if ((value & DockWindowFlags::NoCloseButton) != DockWindowFlags::Undefined)
				flags |= ImGuiDockNodeFlags_NoCloseButton;
			if ((value & DockWindowFlags::NoDockingOverMe) != DockWindowFlags::Undefined)
				flags |= ImGuiDockNodeFlags_NoDockingOverMe;
			if ((value & DockWindowFlags::NoDockingOverOther) != DockWindowFlags::Undefined)
				flags |= ImGuiDockNodeFlags_NoDockingOverOther;
			if ((value & DockWindowFlags::NoDockingOverEmpty) != DockWindowFlags::Undefined)
				flags |= ImGuiDockNodeFlags_NoDockingOverEmpty;
			return flags;
		}

		ImGuiTabItemFlags to_imgui_dock_tab_flags(DockTabFlags value)
		{
			ImGuiTabItemFlags flags = 0;
			if ((value & DockTabFlags::Unsaved) != DockTabFlags::Undefined)
				flags |= ImGuiTabItemFlags_UnsavedDocument;
			if ((value & DockTabFlags::SetSelected) != DockTabFlags::Undefined)
				flags |= ImGuiTabItemFlags_SetSelected;
			if ((value & DockTabFlags::NoCloseWithMiddleMouseButton) != DockTabFlags::Undefined)
				flags |= ImGuiTabItemFlags_NoCloseWithMiddleMouseButton;
			if ((value & DockTabFlags::NoTooltip) != DockTabFlags::Undefined)
				flags |= ImGuiTabItemFlags_NoTooltip;
			if ((value & DockTabFlags::NoReorder) != DockTabFlags::Undefined)
				flags |= ImGuiTabItemFlags_NoReorder;
			if ((value & DockTabFlags::Leading) != DockTabFlags::Undefined)
				flags |= ImGuiTabItemFlags_Leading;
			if ((value & DockTabFlags::Trailing) != DockTabFlags::Undefined)
				flags |= ImGuiTabItemFlags_Trailing;
			if ((value & DockTabFlags::NoAssumedClosure) != DockTabFlags::Undefined)
				flags |= ImGuiTabItemFlags_NoAssumedClosure;
			return flags;
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

		ImGuiDockNodeFlags to_imgui_dock_node_flags(DockNodeFlags value)
		{
			return static_cast<ImGuiDockNodeFlags>(value);
		}

		ImGuiDir to_imgui_dock_dir(DockSplitDir value)
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

		ImGuiKey to_imgui_key(Key value)
		{
			return static_cast<ImGuiKey>(value.value);
		}

		int to_imgui_mouse_button(MouseButton value)
		{
			return static_cast<int>(value.value);
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

		bool has_text(const char* text)
		{
			return text != nullptr && text[0] != '\0';
		}

		bool has_any_bound(const Size& min, const Size& max)
		{
			const Vec2 resolved_min = resolve(min);
			const Vec2 resolved_max = resolve(max);
			return resolved_min.x > 0.0f || resolved_min.y > 0.0f || resolved_max.x > 0.0f || resolved_max.y > 0.0f;
		}

		bool has_window_class_overrides(DockWindowFlags dock_flags, DockTabFlags tab_flags)
		{
			return dock_flags != DockWindowFlags::Undefined || tab_flags != DockTabFlags::Undefined;
		}

		DockPlacement resolve_dock_placement(const WindowOptions& options)
		{
			DockPlacement placement = options.dock;
			if (!placement.id && has_text(placement.id_text))
			{
				placement.id = to_ui_id(ImHashStr(placement.id_text));
			}
			return placement;
		}

		void apply_window_options_pre_begin(const WindowOptions& options)
		{
			if (options.placement.position_condition != Condition::Undefined)
			{
				ImGui::SetNextWindowPos(to_imvec(options.placement.position),
				                        to_imgui_cond(options.placement.position_condition));
			}

			if (options.placement.size_condition != Condition::Undefined)
			{
				ImGui::SetNextWindowSize(to_imvec(resolve(options.placement.size)),
				                         to_imgui_cond(options.placement.size_condition));
			}

			if (has_any_bound(options.placement.min_size, options.placement.max_size))
			{
				const Vec2 resolved_min = resolve(options.placement.min_size);
				const Vec2 resolved_max = resolve(options.placement.max_size);
				const ImVec2 min_size(resolved_min.x > 0.0f ? resolved_min.x : 0.0f,
				                      resolved_min.y > 0.0f ? resolved_min.y : 0.0f);
				const float max_value = std::numeric_limits<float>::max();
				const ImVec2 max_size(resolved_max.x > 0.0f ? resolved_max.x : max_value,
				                      resolved_max.y > 0.0f ? resolved_max.y : max_value);
				ImGui::SetNextWindowSizeConstraints(min_size, max_size);
			}

			if (options.state.collapsed_condition != Condition::Undefined)
			{
				ImGui::SetNextWindowCollapsed(options.state.collapsed, to_imgui_cond(options.state.collapsed_condition));
			}

			if (options.state.focus)
			{
				ImGui::SetNextWindowFocus();
			}

			const DockPlacement dock_placement = resolve_dock_placement(options);
			if (dock_placement.id && dock_placement.condition != Condition::Undefined)
			{
				ImGui::SetNextWindowDockID(to_imgui_id(dock_placement.id), to_imgui_cond(dock_placement.condition));
			}

			if (has_window_class_overrides(dock_placement.flags, options.tab.flags))
			{
				ImGuiWindowClass window_class;
				window_class.DockNodeFlagsOverrideSet = to_imgui_dock_window_flags(dock_placement.flags);
				window_class.TabItemFlagsOverrideSet  = to_imgui_dock_tab_flags(options.tab.flags);
				window_class.DockingAlwaysTabBar =
				        (dock_placement.flags & DockWindowFlags::AlwaysTabBar) != DockWindowFlags::Undefined;
				if ((dock_placement.flags & DockWindowFlags::AllowUnclassed) != DockWindowFlags::Undefined)
				{
					window_class.DockingAllowUnclassed = true;
				}
				ImGui::SetNextWindowClass(&window_class);
			}
		}

		void apply_window_options_post_begin(const WindowOptions& options)
		{
			ImGuiWindow* window = ImGui::GetCurrentWindowRead();
			if (window == nullptr || window->DockNode == nullptr || !window->DockTabIsVisible)
			{
				return;
			}

			const ImRect tab_rect = window->DC.DockTabItemRect;
			if (!tab_rect.IsInverted() && has_color(options.tab.color))
			{
				ImDrawList* draw =
				        window->DockNode->HostWindow != nullptr ? window->DockNode->HostWindow->DrawList : window->DrawList;
				if (draw != nullptr)
				{
					const float inset  = 5.0f;
					const float height = 4.0f;
					const float y      = tab_rect.Min.y + 2.0f;
					const float x1     = tab_rect.Min.x + inset;
					const float x2     = tab_rect.Max.x - inset;
					if (x2 > x1)
					{
						draw->AddRectFilled(ImVec2(x1, y), ImVec2(x2, y + height), col_u32(options.tab.color), height * 0.5f);
					}
				}
			}

			if (has_text(options.tab.tooltip) && ImGui::IsMouseHoveringRect(tab_rect.Min, tab_rect.Max))
			{
				ImGui::SetTooltip("%s", options.tab.tooltip);
			}
		}

		Vec4 mix_color(const Vec4& a, const Vec4& b, float t)
		{
			return Math::lerp(a, b, Math::clamp(t, 0.0f, 1.0f));
		}

		Vec4 color_for_notification_kind(NotificationKind kind)
		{
			switch (kind.value)
			{
				case NotificationKind::Success: return active_context()->style.colors.success;
				case NotificationKind::Warning: return active_context()->style.colors.warning;
				case NotificationKind::Error: return active_context()->style.colors.error;
				case NotificationKind::Info:
				default: return active_context()->style.colors.accent;
			}
		}

		const char* icon_for_notification_kind(NotificationKind kind)
		{
			switch (kind.value)
			{
				case NotificationKind::Success: return ICON_LC_CIRCLE_CHECK;
				case NotificationKind::Warning: return ICON_LC_TRIANGLE_ALERT;
				case NotificationKind::Error: return ICON_LC_CIRCLE_X;
				case NotificationKind::Info:
				default: return ICON_LC_INFO;
			}
		}

		bool blur_visible(const BlurOptions& options)
		{
			return options.radius > 0.0f || options.tint.w > 0.0f;
		}

		bool equals_case_insensitive(const char* a, const char* b)
		{
			if (a == nullptr || b == nullptr)
			{
				return a == b;
			}
			while (*a != '\0' && *b != '\0')
			{
				if (std::tolower(static_cast<unsigned char>(*a)) != std::tolower(static_cast<unsigned char>(*b)))
				{
					return false;
				}
				++a;
				++b;
			}
			return *a == '\0' && *b == '\0';
		}

		bool starts_with_case_insensitive(const char* text, const char* prefix)
		{
			if (!has_text(text) || !has_text(prefix))
			{
				return false;
			}
			while (*prefix != '\0')
			{
				if (*text == '\0' ||
				    std::tolower(static_cast<unsigned char>(*text)) != std::tolower(static_cast<unsigned char>(*prefix)))
				{
					return false;
				}
				++text;
				++prefix;
			}
			return true;
		}

		bool contains_case_insensitive_text(const char* text, const char* query)
		{
			if (!has_text(query))
			{
				return true;
			}
			if (!has_text(text))
			{
				return false;
			}

			for (const char* it = text; *it != '\0'; ++it)
			{
				const char* a = it;
				const char* b = query;
				while (*a != '\0' && *b != '\0' &&
				       std::tolower(static_cast<unsigned char>(*a)) == std::tolower(static_cast<unsigned char>(*b)))
				{
					++a;
					++b;
				}
				if (*b == '\0')
				{
					return true;
				}
			}
			return false;
		}

		int command_match_score(const RegisteredCommand& command, const char* query)
		{
			if (!has_text(query))
			{
				return 0;
			}
			if (equals_case_insensitive(command.name.c_str(), query))
			{
				return 700;
			}
			if (starts_with_case_insensitive(command.name.c_str(), query))
			{
				return 600;
			}
			if (contains_case_insensitive_text(command.name.c_str(), query))
			{
				return 500;
			}
			if (contains_case_insensitive_text(command.description.c_str(), query))
			{
				return 300;
			}
			if (contains_case_insensitive_text(command.id.c_str(), query))
			{
				return 200;
			}
			if (contains_case_insensitive_text(command.shortcut.c_str(), query))
			{
				return 100;
			}
			return -1;
		}

		void refresh_command_palette_results()
		{
			auto& palette = active_context()->command_palette;

			palette.filtered_indices.clear();
			const bool has_query = has_text(palette.search);

			for (usize i = 0, count = palette.commands.size(); i < count; ++i)
			{
				if (command_match_score(palette.commands[i], palette.search) >= 0)
				{
					palette.filtered_indices.push_back(i);
				}
			}

			if (has_query)
			{
				std::stable_sort(palette.filtered_indices.begin(), palette.filtered_indices.end(), [&palette](int a, int b) {
					return command_match_score(palette.commands[a], palette.search) >
					       command_match_score(palette.commands[b], palette.search);
				});
			}

			if (palette.filtered_indices.empty())
			{
				palette.selected_index = 0;
			}
			else
			{
				palette.selected_index = Math::clamp<u32>(palette.selected_index, 0, palette.filtered_indices.size() - 1);
			}
		}

		const Shadow& current_shadow()
		{
			auto& stack = active_context()->shadow_stack;
			return stack.empty() ? active_context()->style.shadow : stack.back();
		}

		const BlurOptions& current_blur()
		{
			auto& stack = active_context()->blur_stack;
			return stack.empty() ? active_context()->style.blur : stack.back();
		}

		bool has_shadow_override()
		{
			return !active_context()->shadow_stack.empty();
		}

		bool shadow_visible(const Shadow& shadow)
		{
			return shadow.color.w > 0.0f;
		}

		Shadow scaled_shadow(const Shadow& shadow, float elevation)
		{
			Shadow result     = shadow;
			const float scale = std::max(0.0f, elevation);
			const float alpha = Math::clamp(elevation, 0.0f, 1.5f);
			result.offset     = Vec2(result.offset.x * scale, result.offset.y * scale);
			result.blur *= scale;
			result.spread *= scale;
			result.color.w *= alpha;
			return result;
		}

		Vec4 with_alpha(Vec4 c, float alpha)
		{
			c.w *= alpha;
			return c;
		}

		Vec4 default_glass_border()
		{
			return Vec4(1.0f, 1.0f, 1.0f, 0.10f);
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

		struct InteractiveRect {
			ImVec2 center        = ImVec2(0.0f, 0.0f);
			ImVec2 visual_size   = ImVec2(0.0f, 0.0f);
			ImVec2 min           = ImVec2(0.0f, 0.0f);
			ImVec2 max           = ImVec2(0.0f, 0.0f);
			float rounding_scale = 1.0f;
		};

		Vec2 make_hover_render_scale(const ImVec2& base_size, float hover_t)
		{
			const Vec2 hover_padding = active_context()->style.hover_padding;

			return Vec2(1.0f + hover_t * hover_padding.x / std::max(1.0f, base_size.x),
			            1.0f + hover_t * hover_padding.y / std::max(1.0f, base_size.y));
		}

		Vec2 make_press_render_scale(const ImVec2& base_size, float active_t)
		{
			const Vec2 press_padding = active_context()->style.press_padding;

			return Vec2(1.0f - active_t * press_padding.x / std::max(1.0f, base_size.x),
			            1.0f - active_t * press_padding.y / std::max(1.0f, base_size.y));
		}

		InteractiveRect make_interactive_rect(const ImVec2& pos, const ImVec2& base_size)
		{
			InteractiveRect rect;
			rect.center      = ImVec2(pos.x + base_size.x * 0.5f, pos.y + base_size.y * 0.5f);
			rect.visual_size = ImVec2(std::max(1.0f, base_size.x), std::max(1.0f, base_size.y));
			rect.min         = ImVec2(rect.center.x - rect.visual_size.x * 0.5f, rect.center.y - rect.visual_size.y * 0.5f);
			rect.max         = ImVec2(rect.center.x + rect.visual_size.x * 0.5f, rect.center.y + rect.visual_size.y * 0.5f);
			rect.rounding_scale =
			        std::min(rect.visual_size.x / std::max(1.0f, base_size.x), rect.visual_size.y / std::max(1.0f, base_size.y));
			return rect;
		}

		void draw_shadow_rect(ImDrawList* draw, const ImVec2& min, const ImVec2& max, float rounding, const Shadow& shadow)
		{
			if (draw == nullptr || !shadow_visible(shadow))
			{
				return;
			}

			const ImVec2 offset = to_imvec(shadow.offset);
			const ImVec2 base_min(min.x + offset.x - shadow.spread, min.y + offset.y - shadow.spread);
			const ImVec2 base_max(max.x + offset.x + shadow.spread, max.y + offset.y + shadow.spread);
			const float base_rounding = std::max(0.0f, rounding + shadow.spread);
			const float blur          = Math::clamp(shadow.blur, 0.0f, 48.0f);
			const float blur_extent   = blur * 0.55f;
			const ImVec2 clip_min(Math::max(draw->GetClipRectMin().x, base_min.x - blur_extent),
			                      Math::max(draw->GetClipRectMin().y, base_min.y - blur_extent));
			const ImVec2 clip_max(Math::min(draw->GetClipRectMax().x, base_max.x + blur_extent),
			                      Math::min(draw->GetClipRectMax().y, base_max.y + blur_extent));
			draw->PushClipRect(clip_min, clip_max, false);

			if (shadow.blur <= 0.0f)
			{
				draw->AddRectFilled(base_min, base_max, col_u32(shadow.color), base_rounding);
				draw->PopClipRect();
				return;
			}

			const int layers = Math::clamp(static_cast<i32>(std::ceil(blur / 4.0f)), 2, 10);
			for (int i = layers - 1; i >= 0; --i)
			{
				const float t     = static_cast<float>(i + 1) / static_cast<float>(layers);
				const float grow  = blur * t * 0.55f;
				const float alpha = (1.0f - t) * (1.0f - t) * 0.95f / static_cast<float>(layers);
				Vec4 layer_color  = shadow.color;
				layer_color.w *= alpha * static_cast<float>(layers);
				draw->AddRectFilled(ImVec2(base_min.x - grow, base_min.y - grow), ImVec2(base_max.x + grow, base_max.y + grow),
				                    col_u32(layer_color), base_rounding + grow);
			}
			draw->PopClipRect();
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

		bool is_modifier_key(ui::Key key)
		{
			return key == ui::Key::LeftCtrl || key == ui::Key::RightCtrl || key == ui::Key::LeftShift ||
			       key == ui::Key::RightShift || key == ui::Key::LeftAlt || key == ui::Key::RightAlt ||
			       key == ui::Key::LeftSuper || key == ui::Key::RightSuper;
		}

		bool keybind_mods_match(const ui::Keybind& binding)
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

			auto& modals = active_context()->pending_modals;

			for (auto it = modals.begin(); it != modals.end(); ++it)
			{
				if (*it == name)
				{
					modals.erase(it);
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

			auto& popups = active_context()->pending_popups;

			for (auto it = popups.begin(); it != popups.end(); ++it)
			{
				if (*it == name)
				{
					popups.erase(it);
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

		void push_input_frame_styles(float focus)
		{
			ImGui::PushStyleColor(ImGuiCol_Border, to_imvec(Math::lerp(active_context()->style.colors.border,
			                                                           active_context()->style.colors.accent, focus)));
		}

		void pop_input_frame_styles()
		{
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
			const bool hovered         = !disabled && ImGui::IsItemHovered();
			const bool active          = !disabled && ImGui::IsItemActive();
			const bool clicked         = !disabled && ImGui::IsItemClicked();
			const bool visual_selected = selected || clicked;
			const float visual_arrow_t = clicked ? (selected ? 0.0f : 1.0f) : arrow_t;

			anim.hover    = approach(anim.hover, hovered ? 1.0f : 0.0f);
			anim.active   = approach(anim.active, active ? 1.0f : 0.0f * 1.6f);
			anim.selected = approach(anim.selected, visual_selected ? 1.0f : 0.0f);

			const Vec4 base = Math::lerp(active_context()->style.colors.panel, active_context()->style.colors.background_hovered,
			                             anim.hover);
			const Vec4 sel  = has_color(accent) ? accent : active_context()->style.colors.accent;
			Vec4 bg         = Math::lerp(base, with_alpha(sel, 0.22f), anim.selected);
			bg              = Math::lerp(bg, with_alpha(sel, 0.18f), anim.active * 0.45f);

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
				draw_chevron(draw, ImVec2(x + 5.0f, cy), 10.0f, visual_arrow_t,
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

		Vec4 notification_color(ui::NotificationKind kind)
		{
			switch (kind)
			{
				case ui::NotificationKind::Success: return active_context()->style.colors.success;
				case ui::NotificationKind::Warning: return active_context()->style.colors.warning;
				case ui::NotificationKind::Error: return active_context()->style.colors.error;
				case ui::NotificationKind::Info:
				default: return active_context()->style.colors.accent;
			}
		}

		void push_menu_bar_colors()
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, active_context()->style.rounding * 0.55f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
			                    ImVec2(active_context()->style.spacing, active_context()->style.spacing * 0.75f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
			                      to_imvec(with_alpha(active_context()->style.colors.accent_hovered, 0.18f)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive,
			                      to_imvec(with_alpha(active_context()->style.colors.accent_active, 0.24f)));
		}

		void pop_menu_bar_colors()
		{
			ImGui::PopStyleColor(2);
			ImGui::PopStyleVar(2);
		}

		void push_menu_popup_colors()
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, active_context()->style.rounding * 0.55f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
			                    ImVec2(active_context()->style.spacing, active_context()->style.spacing * 0.6f));
		}

		void pop_menu_popup_colors()
		{
			ImGui::PopStyleVar(2);
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
			ImGui::PushStyleColor(ImGuiCol_WindowBg, to_imvec(bg));
			ImGui::PushStyleColor(ImGuiCol_PopupBg, to_imvec(bg));
			ImGui::PushStyleColor(ImGuiCol_TitleBg, to_imvec(title_bg));
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, to_imvec(title_bg_active));
			ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, to_imvec(title_bg));
		}

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
	}// namespace
}// namespace Trinex::UI
