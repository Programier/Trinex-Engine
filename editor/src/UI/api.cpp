#include <Core/editor_config.hpp>
#include <Core/etl/vector.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Graphics/render_pools.hpp>
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
#include <utility>

#include <Graphics/render_viewport.hpp>

namespace Trinex::UI
{
	class Allocator
	{
	private:
		static constexpr usize block_size = 4096;

		struct Block {
			alignas(block_size) u8 data[block_size];
			Block* next = nullptr;
		};

		Block* m_head    = nullptr;
		Block* m_current = nullptr;
		usize m_used     = 0;

		Block* create_block()
		{
			Block* block = trx_new Block{};

			if (!m_head)
			{
				m_head = block;
			}

			if (m_current)
			{
				m_current->next = block;
			}

			m_current = block;
			m_used    = 0;
			return block;
		}

	public:
		void* allocate(usize size)
		{
			if (size == 0)
			{
				return nullptr;
			}

			size = align_up(size, 16);

			if (size > block_size)
			{
				trinex_unreachable();
			}

			if (!m_current)
			{
				create_block();
			}

			if (m_used + size > block_size)
			{
				if (m_current->next)
				{
					m_current = m_current->next;
					m_used    = 0;
				}
				else
				{
					create_block();
				}
			}

			void* ptr = m_current->data + m_used;
			m_used += size;
			return ptr;
		}

		void* allocate(usize size, void* memory)
		{
			void* ptr = allocate(size);
			memcpy(ptr, memory, size);
			return ptr;
		}

		void reset()
		{
			m_current = m_head;
			m_used    = 0;
		}

		~Allocator()
		{
			Block* block = m_head;

			while (block)
			{
				Block* next = block->next;
				trx_delete block;
				block = next;
			}
		}
	};

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
		String name;
		WindowFlags flags = WindowFlags::Undefined;
		WindowOptions options;
		bool open = true;
		Function<void()> content;
	};

	struct RegisteredCommand {
		String id;
		String name;
		String description;
		String shortcut;
		String icon;
		Function<void()> action;
	};

	struct CommandPaletteState {
		bool open                    = false;
		bool focus_search_next_frame = false;
		char search[256]             = {};
		int selected_index           = 0;
		Vector<RegisteredCommand> commands;
		Vector<int> filtered_indices;
	};

	struct ActiveWindowScope {
		bool* external_open          = nullptr;
		PersistentWindow* persistent = nullptr;
	};

	struct Context {
		Trinex::Window* window = nullptr;
		ImGuiContext* context  = nullptr;
		Allocator allocator;
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
		Vector<PanelContext> panel_stack;
		Vector<GlassPanelContext> glass_panel_stack;
		Vector<CardContext> card_stack;
		Vector<Shadow> shadow_stack;
		Vector<BlurOptions> blur_stack;
		Vector<float> disabled_alpha_stack;
		Vector<String> pending_modals;
		Vector<String> pending_popups;
		Vector<Notification> notifications;
		Vector<PersistentWindow> windows;
		CommandPaletteState command_palette;
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
		using tree_context = TreeContext;
		using area_context = AreaContext;

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
#define g_panel_stack active_context()->panel_stack
#define g_glass_panel_stack active_context()->glass_panel_stack
#define g_card_stack active_context()->card_stack
#define g_shadow_stack active_context()->shadow_stack
#define g_blur_stack active_context()->blur_stack
#define g_disabled_alpha_stack active_context()->disabled_alpha_stack
#define g_pending_modals active_context()->pending_modals
#define g_pending_popups active_context()->pending_popups
#define g_command_palette active_context()->command_palette
#define g_notifications active_context()->notifications

		float dt()
		{
			return std::max(0.0f, ImGui::GetIO().DeltaTime);
		}

		void* memory_copy(void* userdata, usize userdata_size)
		{
			if (userdata == nullptr || userdata_size == 0)
			{
				return userdata;
			}

			return active_context()->allocator.allocate(userdata_size, userdata);
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
				ctx->viewport(Trinex::RHIViewport(args->size, args->pos));
				ctx->scissor(Trinex::RHIScissor(args->size, args->pos));
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

		ImDrawList* resolve_draw_list(DrawList draw_list, ImGuiWindow* window, ImGuiViewport*& viewport)
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

		PersistentWindow* find_window(const char* name)
		{
			if (name == nullptr)
			{
				return nullptr;
			}

			for (PersistentWindow& window : active_context()->windows)
			{
				if (window.name == name)
				{
					return &window;
				}
			}
			return nullptr;
		}

		PersistentWindow& ensure_window(const char* name)
		{
			if (PersistentWindow* window = find_window(name))
			{
				return *window;
			}

			PersistentWindow& window = active_context()->windows.emplace_back();
			window.name              = name;
			return window;
		}

		void setup_window(PersistentWindow& window, WindowFlags flags, const WindowOptions& options, bool open,
		                  const Function<void()>& content)
		{
			window.flags   = flags;
			window.options = options;
			window.open    = open;
			window.content = content;
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

		int to_imgui_MouseButton(MouseButton value)
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

		bool has_any_bound(const Vec2& min, const Vec2& max)
		{
			return min.x > 0.0f || min.y > 0.0f || max.x > 0.0f || max.y > 0.0f;
		}

		bool has_window_class_overrides(const WindowOptions& options)
		{
			return options.dock_flags != DockWindowFlags::Undefined || options.tab_flags != DockTabFlags::Undefined;
		}

		void apply_window_options_pre_begin(const WindowOptions& options)
		{
			if (options.position_condition != Condition::Undefined)
			{
				ImGui::SetNextWindowPos(to_imvec(options.position), to_imgui_cond(options.position_condition));
			}

			if (options.size_condition != Condition::Undefined)
			{
				ImGui::SetNextWindowSize(to_imvec(options.size), to_imgui_cond(options.size_condition));
			}

			if (has_any_bound(options.min_size, options.max_size))
			{
				const ImVec2 min_size(options.min_size.x > 0.0f ? options.min_size.x : 0.0f,
				                      options.min_size.y > 0.0f ? options.min_size.y : 0.0f);
				const float max_value = std::numeric_limits<float>::max();
				const ImVec2 max_size(options.max_size.x > 0.0f ? options.max_size.x : max_value,
				                      options.max_size.y > 0.0f ? options.max_size.y : max_value);
				ImGui::SetNextWindowSizeConstraints(min_size, max_size);
			}

			if (options.collapsed_condition != Condition::Undefined)
			{
				ImGui::SetNextWindowCollapsed(options.collapsed, to_imgui_cond(options.collapsed_condition));
			}

			if (options.focus)
			{
				ImGui::SetNextWindowFocus();
			}

			if (options.dock_id && options.dock_condition != Condition::Undefined)
			{
				ImGui::SetNextWindowDockID(to_imgui_id(options.dock_id), to_imgui_cond(options.dock_condition));
			}

			if (has_window_class_overrides(options))
			{
				ImGuiWindowClass window_class;
				window_class.DockNodeFlagsOverrideSet = to_imgui_dock_window_flags(options.dock_flags);
				window_class.TabItemFlagsOverrideSet  = to_imgui_dock_tab_flags(options.tab_flags);
				window_class.DockingAlwaysTabBar =
				        (options.dock_flags & DockWindowFlags::AlwaysTabBar) != DockWindowFlags::Undefined;
				if ((options.dock_flags & DockWindowFlags::AllowUnclassed) != DockWindowFlags::Undefined)
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
			if (!tab_rect.IsInverted() && has_color(options.tab_color))
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
						draw->AddRectFilled(ImVec2(x1, y), ImVec2(x2, y + height), col_u32(options.tab_color), height * 0.5f);
					}
				}
			}

			if (has_text(options.tab_tooltip) && ImGui::IsMouseHoveringRect(tab_rect.Min, tab_rect.Max))
			{
				ImGui::SetTooltip("%s", options.tab_tooltip);
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
			g_command_palette.filtered_indices.clear();
			const bool has_query = has_text(g_command_palette.search);

			for (int i = 0; i < static_cast<int>(g_command_palette.commands.size()); ++i)
			{
				if (command_match_score(g_command_palette.commands[i], g_command_palette.search) >= 0)
				{
					g_command_palette.filtered_indices.push_back(i);
				}
			}

			if (has_query)
			{
				std::stable_sort(g_command_palette.filtered_indices.begin(), g_command_palette.filtered_indices.end(),
				                 [](int a, int b) {
					                 return command_match_score(g_command_palette.commands[a], g_command_palette.search) >
					                        command_match_score(g_command_palette.commands[b], g_command_palette.search);
				                 });
			}

			if (g_command_palette.filtered_indices.empty())
			{
				g_command_palette.selected_index = 0;
			}
			else
			{
				g_command_palette.selected_index = Math::clamp(g_command_palette.selected_index, 0,
				                                               static_cast<int>(g_command_palette.filtered_indices.size()) - 1);
			}
		}

		const Shadow& current_shadow()
		{
			return g_shadow_stack.empty() ? active_context()->style.shadow : g_shadow_stack.back();
		}

		const BlurOptions& current_blur()
		{
			return g_blur_stack.empty() ? active_context()->style.blur : g_blur_stack.back();
		}

		bool has_shadow_override()
		{
			return !g_shadow_stack.empty();
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

		InteractiveRect make_interactive_rect(const ImVec2& pos, const ImVec2& base_size, float hover_t, float active_t)
		{
			const Vec2 hover_padding = active_context()->style.hover_padding;
			const Vec2 press_padding(3.0f, 3.0f);

			InteractiveRect rect;
			rect.center      = ImVec2(pos.x + base_size.x * 0.5f, pos.y + base_size.y * 0.5f);
			rect.visual_size = ImVec2(std::max(1.0f, base_size.x + hover_t * hover_padding.x - active_t * press_padding.x),
			                          std::max(1.0f, base_size.y + hover_t * hover_padding.y - active_t * press_padding.y));
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
			draw->PushClipRectFullScreen();

			if (shadow.blur <= 0.0f)
			{
				draw->AddRectFilled(base_min, base_max, col_u32(shadow.color), base_rounding);
				draw->PopClipRect();
				return;
			}

			const float blur = Math::clamp(shadow.blur, 0.0f, 48.0f);
			const int layers = Math::clamp(static_cast<int>(std::ceil(blur / 4.0f)), 2, 10);
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

		void push_input_frame_styles(float focus)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, active_context()->style.rounding);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(active_context()->style.padding, 6.0f));
			ImGui::PushStyleColor(ImGuiCol_FrameBg, to_imvec(active_context()->style.colors.background));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, to_imvec(active_context()->style.colors.background_hovered));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, to_imvec(active_context()->style.colors.background_active));
			ImGui::PushStyleColor(ImGuiCol_Border, to_imvec(Math::lerp(active_context()->style.colors.border,
			                                                           active_context()->style.colors.accent, focus)));
		}

		void pop_input_frame_styles()
		{
			ImGui::PopStyleColor(4);
			ImGui::PopStyleVar(2);
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

			anim.hover    = approach(anim.hover, hovered ? 1.0f : 0.0f, active_context()->style.animation_speed);
			anim.active   = approach(anim.active, active ? 1.0f : 0.0f, active_context()->style.animation_speed * 1.6f);
			anim.selected = approach(anim.selected, visual_selected ? 1.0f : 0.0f, active_context()->style.animation_speed);

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
			const ImVec2 origin(viewport->WorkPos.x + viewport->WorkSize.x - 16.0f,
			                    viewport->WorkPos.y + viewport->WorkSize.y - 16.0f);
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
				const ImVec2 max(origin.x - slide, animated_y);
				const ImVec2 min(max.x - width, animated_y - height);
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
				target_y += -(height + 10.0f);
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
			for (PersistentWindow& window : active_context()->windows)
			{
				if (!window.open)
				{
					continue;
				}

				active_context()->rendering_window = &window;
				const bool visible = begin_window(window.name.c_str(), &window.open, window.flags, window.options);
				active_context()->rendering_window = nullptr;

				if (visible && window.content)
				{
					window.content();
					end_window();
				}
			}
		}
	}// namespace

	/////////////////////// LIFECYCLE AND FRAME ///////////////////////

	void initialize()
	{
		active_context()->style = Style{};
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
		g_panel_stack.clear();
		g_glass_panel_stack.clear();
		g_card_stack.clear();
		g_shadow_stack.clear();
		g_disabled_alpha_stack.clear();
		g_pending_modals.clear();
		g_pending_popups.clear();
		g_command_palette.commands.clear();
		g_command_palette.filtered_indices.clear();
		g_command_palette.open                    = false;
		g_command_palette.focus_search_next_frame = false;
		g_command_palette.search[0]               = '\0';
		g_command_palette.selected_index          = 0;
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
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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

	bool begin_frame(Context* context)
	{
		trinex_assert(context);
		g_context = context;

		ImGui::SetCurrentContext(context->context);
		UI::Backend::imgui_new_frame(context->window);
		ImGui::NewFrame();

		return true;
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

				UI::Backend::imgui_render(ctx, g_context->window, ImGui::GetDrawData());

				ctx->barrier(texture, RHIAccess::PresentSrc);
			}

			RHIContextPool::global_instance()->end_context(ctx, swapchain->acquire_semaphore(), swapchain->present_semaphore());
			RHI::instance()->present(swapchain);
		}

		trinex_assert(g_shadow_stack.empty() && "UI::push_shadow()/pop_shadow() imbalance detected at end_frame()");
		if (!g_shadow_stack.empty())
		{
			g_shadow_stack.clear();
		}

		trinex_assert(g_glass_panel_stack.empty() &&
		              "UI::begin_glass_panel()/end_glass_panel() imbalance detected at end_frame()");
		if (!g_glass_panel_stack.empty())
		{
			g_glass_panel_stack.clear();
		}

		active_context()->allocator.reset();
		ImGui::SetCurrentContext(nullptr);
	}

	/////////////////////// STYLE AND EFFECTS ///////////////////////

	Style& get_style()
	{
		return active_context()->style;
	}

	void set_style(const Style& value)
	{
		active_context()->style = value;
	}

	void push_style(const Style& value)
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

	void push_shadow(const Shadow& shadow)
	{
		g_shadow_stack.push_back(shadow);
	}

	void pop_shadow()
	{
		trinex_assert(!g_shadow_stack.empty() && "UI::pop_shadow() called without matching push_shadow()");
		if (!g_shadow_stack.empty())
		{
			g_shadow_stack.pop_back();
		}
	}

	void push_blur(const BlurOptions& options)
	{
		g_blur_stack.push_back(options);
	}

	void pop_blur()
	{
		trinex_assert(!g_blur_stack.empty() && "UI::pop_blur() called without matching push_blur()");
		if (!g_blur_stack.empty())
		{
			g_blur_stack.pop_back();
		}
	}

	void blur(const Vec2& min, const Vec2& max, DrawList draw_list, const BlurOptions& options)
	{
		const float spread = std::max(0.0f, options.spread);
		const Vec2 pad(spread, spread);
		const Vec2 area_min  = min - pad;
		const Vec2 area_max  = max + pad;
		const Vec2 area_size = area_max - area_min;

		if (options.radius > 0.0f)
		{
			paint(draw_list, [options, area_min, area_max]() {
				const float radius = Math::clamp(options.radius, 0.0f, 64.0f);

				if (radius <= 0.0f)
				{
					return;
				}

				const float sigma           = options.sigma > 0.0f ? options.sigma : std::max(1.0f, radius * 0.45f);
				const RHITextureFlags flags = RHITextureFlags::ColorAttachment;
				RHITexturePool* pool        = RHITexturePool::global_instance();

				RHIContext* ctx              = Backend::rhi();
				RHITexture* window           = Backend::render_target();
				const Vector2u viewport_size = window->size();
				RHITexture* temporary        = pool->request_surface(RHISurfaceFormat::RGBA8, viewport_size, flags);

				const Vector2f blur_offset = area_min / Vector2f(viewport_size);
				const Vector2f blur_size   = (area_max - area_min) / Vector2f(viewport_size);

				ctx->push_debug_stage("Bloor");

				ctx->end_rendering();
				ctx->barrier(window, RHIAccess::SRVGraphics);
				ctx->barrier(temporary, RHIAccess::RTV);

				ctx->begin_rendering(temporary->as_rtv());
				Pipelines::GaussianBlur::blur(ctx, window->as_srv(), {0.f, 1.f / static_cast<f32>(viewport_size.y)}, sigma,
				                              radius, {}, nullptr, blur_offset, blur_size);
				ctx->end_rendering();

				ctx->barrier(window, RHIAccess::RTV);
				ctx->barrier(temporary, RHIAccess::SRVGraphics);
				ctx->begin_rendering(window->as_rtv());
				Pipelines::GaussianBlur::blur(ctx, temporary->as_srv(), {1.f / static_cast<f32>(viewport_size.x), 0.f}, sigma,
				                              radius, {}, nullptr, blur_offset, blur_size);

				if (options.noise_opacity > 0.f)
				{
					ctx->push_debug_stage("Noise");
					Pipelines::NoiseApplication::noise(ctx, options.noise_opacity, options.noise_scale, blur_offset, blur_size);
					ctx->pop_debug_stage();
				}

				ctx->pop_debug_stage();


				pool->return_surface(temporary);
			});
		}

		if (options.tint.w > 0.0f)
		{
			ImGuiWindow* window     = ImGui::GetCurrentWindow();
			ImGuiViewport* viewport = nullptr;
			ImDrawList* list        = resolve_draw_list(draw_list, window, viewport);
			if (list != nullptr)
			{
				const float base_rounding = options.rounding >= 0.0f ? options.rounding : active_context()->style.rounding;
				list->AddRectFilled(to_imvec(area_min), to_imvec(area_max), col_u32(options.tint),
				                    std::max(0.0f, base_rounding + spread));
			}
		}
	}

	void paint(Vec2 pos, Vec2 size, PaintFunction function, void* userdata, usize userdata_size, DrawList draw_list)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		if (function == nullptr)
			return;

		ImGuiViewport* viewport = nullptr;
		ImDrawList* list        = resolve_draw_list(draw_list, window, viewport);
		add_paint_callback(list, viewport, pos, size, function, userdata, userdata_size);
	}

	void paint(Vec2 size, PaintFunction function, void* userdata, usize userdata_size, DrawList draw_list)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiViewport* vp   = window->Viewport;

		Vec2 pos = to_vec(ImGui::GetItemRectMin());
		paint(pos, size, function, userdata, userdata_size, draw_list);
	}

	void paint(PaintFunction function, void* userdata, usize userdata_size, DrawList draw_list)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiViewport* vp   = window ? window->Viewport : ImGui::GetMainViewport();

		Vec2 pos  = to_vec(vp->Pos);
		Vec2 size = to_vec(vp->Size);
		paint(pos, size, function, userdata, userdata_size, draw_list);
	}

	/////////////////////// ANIMATION AND IDENTITY ///////////////////////

	float apply_ease(float t, Ease mode)
	{
		t = Math::clamp(t, 0.f, 1.f);
		switch (mode)
		{
			case Ease::Linear: return t;
			case Ease::InQuad: return t * t;
			case Ease::OutQuad: return 1.0f - (1.0f - t) * (1.0f - t);
			case Ease::InOutQuad: return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f;
			case Ease::OutBack:
			{
				const float c1 = 1.70158f;
				const float c3 = c1 + 1.0f;
				return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
			}
			case Ease::OutCubic:
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

	/////////////////////// DOCKING ///////////////////////

	bool begin_dockspace(const char* id_text, const DockspaceOptions& options)
	{
		trinex_assert(id_text != nullptr && "UI::begin_dockspace() requires a valid id");

		ImGuiViewport* viewport       = ImGui::GetMainViewport();
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		                                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		                                ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
		                                ImGuiWindowFlags_NoSavedSettings;

		ImGuiDockNodeFlags dock_flags = to_imgui_dock_node_flags(options.flags);
		if ((dock_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0 || !options.background)
		{
			window_flags |= ImGuiWindowFlags_NoBackground;
		}

		const ImVec2 dockspace_size(options.size.x > 0.0f ? options.size.x : viewport->WorkSize.x,
		                            options.size.y > 0.0f ? options.size.y : viewport->WorkSize.y);

		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(dockspace_size);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, options.border ? active_context()->style.border_size : 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		char host_name[256];
		std::snprintf(host_name, sizeof(host_name), "##dockspace_host_%s", id_text);

		const bool visible = ImGui::Begin(host_name, nullptr, window_flags);
		if (!visible)
		{
			ImGui::End();
			ImGui::PopStyleVar(3);
			return false;
		}

		ImGui::DockSpace(to_imgui_id(dockspace_id(id_text)), ImVec2(0.0f, 0.0f), dock_flags);
		return true;
	}

	void end_dockspace()
	{
		ImGui::End();
		ImGui::PopStyleVar(3);
	}

	ID dockspace_id(const char* id_text)
	{
		if (id_text == nullptr)
		{
			return ID(0);
		}

		return to_ui_id(ImHashStr(id_text));
	}

	void set_next_window_dock(ID dock_id, Condition condition)
	{
		ImGui::SetNextWindowDockID(to_imgui_id(dock_id), to_imgui_cond(condition));
	}

	void set_next_window_dock(const char* dockspace_id_text, Condition condition)
	{
		set_next_window_dock(dockspace_id(dockspace_id_text), condition);
	}

	bool is_window_docked()
	{
		return ImGui::IsWindowDocked();
	}

	ID window_dock_id()
	{
		return to_ui_id(ImGui::GetWindowDockID());
	}

	void undock_window()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindowRead();
		if (window != nullptr && window->DockNode != nullptr)
		{
			ImGui::DockContextQueueUndockWindow(ImGui::GetCurrentContext(), window);
		}
	}

	void dock_builder_begin(ID dockspace_id, const Vec2& size, DockNodeFlags flags)
	{
		const ImGuiID root_id      = to_imgui_id(dockspace_id);
		ImGuiViewport* viewport    = ImGui::GetMainViewport();
		const ImVec2 fallback_size = viewport != nullptr ? viewport->WorkSize : ImVec2(0.0f, 0.0f);
		const ImVec2 resolved_size(size.x > 0.0f ? size.x : fallback_size.x, size.y > 0.0f ? size.y : fallback_size.y);

		ImGui::DockBuilderRemoveNode(root_id);
		ImGui::DockBuilderAddNode(root_id, to_imgui_dock_node_flags(flags) | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(root_id, resolved_size);
	}

	void dock_builder_end() {}

	void dock_builder_clear(ID dockspace_id)
	{
		ImGui::DockBuilderRemoveNode(to_imgui_id(dockspace_id));
	}

	void dock_builder_set_flags(ID dock_id, DockNodeFlags flags)
	{
		if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(to_imgui_id(dock_id)))
		{
			node->SetLocalFlags(to_imgui_dock_node_flags(flags));
		}
	}

	DockBuilderSplitResult dock_builder_split(ID dock_id, DockSplitDir dir, float ratio)
	{
		DockBuilderSplitResult result;
		ImGuiID parent_id = 0;
		ImGuiID child_id  = 0;

		ImGui::DockBuilderSplitNode(to_imgui_id(dock_id), to_imgui_dock_dir(dir), Math::clamp(ratio, 0.0f, 1.0f), &child_id,
		                            &parent_id);

		result.parent = to_ui_id(parent_id);
		result.child  = to_ui_id(child_id);
		return result;
	}

	void dock_builder_dock_window(const char* window_name, ID dock_id)
	{
		ImGui::DockBuilderDockWindow(window_name, to_imgui_id(dock_id));
	}

	void dock_builder_finish(ID dockspace_id)
	{
		ImGui::DockBuilderFinish(to_imgui_id(dockspace_id));
	}

	void build_dockspace_once(const char* id_text, const FunctionRef<void(ID root_dock)>& builder)
	{
		const ID root = dockspace_id(id_text);
		if (ImGui::DockBuilderGetNode(to_imgui_id(root)) == nullptr)
		{
			builder(root);
		}
	}

	/////////////////////// WINDOWS AND CONTAINERS ///////////////////////

	bool begin_window(const char* name, bool* open, WindowFlags flags, const WindowOptions& options)
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
		ActiveWindowScope scope;
		scope.external_open = open;
		scope.persistent    = active_context()->rendering_window;
		active_context()->active_window_stack.push_back(scope);
		const bool visible = ImGui::Begin(name, open, to_imgui_window_flags(flags));
		if (visible)
		{
			apply_window_options_post_begin(options);
		}
		if (!visible)
		{
			ImGui::End();
			active_context()->active_window_stack.pop_back();
			pop_window_styles();
		}
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

	void create_window(const char* name, WindowFlags flags, const WindowOptions& options, const Function<void()>& content)
	{
		if (name == nullptr || content == nullptr)
		{
			return;
		}

		setup_window(ensure_window(name), flags, options, true, content);
	}

	bool is_window_open(const char* name)
	{
		if (PersistentWindow* window = find_window(name))
		{
			return window->open;
		}
		return false;
	}

	void open_window(const char* name)
	{
		if (PersistentWindow* window = find_window(name))
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
		if (PersistentWindow* window = find_window(name))
		{
			window->open = false;
		}
	}

	bool begin_panel(const char* id, const PanelOptions& options)
	{
		const Vec4 bg = has_color(options.background_color) ? options.background_color : active_context()->style.colors.panel;
		const float rounding = options.rounding >= 0.0f ? options.rounding : active_context()->style.rounding;
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, rounding);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
		                    ImVec2(active_context()->style.padding, active_context()->style.padding));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
		const bool visible =
		        ImGui::BeginChild(id, to_imvec(options.size), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_None);
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
		g_panel_stack.push_back(context);
		ImGui::GetWindowDrawList()->ChannelsSplit(2);
		ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);
		return true;
	}

	void end_panel()
	{
		trinex_assert(!g_panel_stack.empty() && "UI::end_panel() called without matching begin_panel()");
		if (g_panel_stack.empty())
		{
			return;
		}

		const PanelContext context = g_panel_stack.back();
		g_panel_stack.pop_back();

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

	bool begin_glass_panel(const char* id, const Vec2& size, const GlassOptions& options)
	{
		trinex_assert(has_text(id) && "UI::begin_glass_panel() requires a non-empty id");
		if (!has_text(id))
		{
			return false;
		}

		const float rounding = options.rounding >= 0.0f ? options.rounding : active_context()->style.rounding;
		const float padding  = options.padding >= 0.0f ? options.padding : active_context()->style.padding;
		const float opacity  = Math::clamp(options.opacity, 0.0f, 1.0f);

		Vec2 resolved_size = size;
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

		const bool visible = ImGui::BeginChild(id, to_imvec(resolved_size), child_flags, ImGuiWindowFlags_None);
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
		context.blur = current_blur();
		context.blur.radius *= opacity;
		context.blur.sigma *= opacity;
		context.blur.spread *= opacity;
		context.blur.rounding = rounding;
		context.shadow        = current_shadow();
		context.shadow.color.w *= opacity;
		g_glass_panel_stack.push_back(context);
		ImGui::GetWindowDrawList()->ChannelsSplit(2);
		ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);
		return true;
	}

	void end_glass_panel()
	{
		trinex_assert(!g_glass_panel_stack.empty() && "UI::end_glass_panel() called without matching begin_glass_panel()");
		if (g_glass_panel_stack.empty())
		{
			return;
		}

		const GlassPanelContext context = g_glass_panel_stack.back();
		g_glass_panel_stack.pop_back();

		const ImVec2 min = ImGui::GetWindowPos();
		const ImVec2 max = add(min, ImGui::GetWindowSize());
		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->ChannelsSetCurrent(0);

		if (context.draw_shadow)
		{
			draw_shadow_rect(draw, min, max, context.rounding, context.shadow);
		}

		if (context.blur.radius > 0.0f)
		{
			blur(to_vec(min), to_vec(max), context.blur);
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
			const float inset = std::max(1.0f, active_context()->style.border_size);
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

	bool begin_child_panel(const char* id, const Vec2& size, const PanelOptions& options)
	{
		PanelOptions copy = options;
		copy.size         = size;
		return begin_panel(id, copy);
	}

	void end_child_panel()
	{
		end_panel();
	}

	bool begin_group_panel(const char* label, const Vec2& size, const PanelOptions& options)
	{
		ImGui::BeginGroup();
		if (label != nullptr && label[0] != '\0')
		{
			text_muted("%s", label);
		}

		if (!begin_child_panel(label, size, options))
		{
			ImGui::EndGroup();
			return false;
		}

		return true;
	}

	void end_group_panel()
	{
		end_child_panel();
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

	bool begin_card(const char* title, const CardOptions& options)
	{
		cleanup_states();
		if (title != nullptr && title[0] != '\0')
		{
			ImGui::PushID(title);
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

		Vec2 size = options.size;
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
		context.elevation        = std::max(0.0f, options.elevation);
		context.accent           = accent;
		context.background_color = bg;
		context.border_color     = border;
		context.shadow           = current_shadow();
		g_card_stack.push_back(context);
		ImGui::GetWindowDrawList()->ChannelsSplit(2);
		ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);

		if (options.disabled)
		{
			begin_disabled(true);
		}

		const bool has_title    = title != nullptr && visible_label(title)[0] != '\0';
		const bool has_subtitle = options.subtitle != nullptr && options.subtitle[0] != '\0';
		const bool has_icon     = options.icon != nullptr && options.icon[0] != '\0';
		const bool has_right    = options.right_text != nullptr && options.right_text[0] != '\0';
		if (has_title || has_subtitle || has_icon || has_right)
		{
			const ImVec2 start     = ImGui::GetCursorScreenPos();
			const float content_w  = ImGui::GetContentRegionAvail().x;
			const float icon_w     = has_icon ? ImGui::CalcTextSize(options.icon).x : 0.0f;
			const float right_w    = has_right ? ImGui::CalcTextSize(options.right_text).x : 0.0f;
			const float title_h    = has_title ? ImGui::GetTextLineHeight() : 0.0f;
			const float subtitle_h = has_subtitle ? ImGui::GetTextLineHeight() : 0.0f;
			const float header_h   = std::max(
                    std::max(title_h + (has_subtitle ? subtitle_h + 2.0f : 0.0f), has_icon ? ImGui::GetTextLineHeight() : 0.0f),
                    has_right ? ImGui::GetTextLineHeight() : 0.0f);
			const float icon_gap   = has_icon ? spacing * 0.75f : 0.0f;
			const float right_gap  = has_right ? spacing : 0.0f;
			const float title_x    = start.x + icon_w + icon_gap;
			const float right_x    = start.x + content_w - right_w;
			ImDrawList* draw       = ImGui::GetWindowDrawList();
			const Vec4 title_color = options.selected ? Math::lerp(active_context()->style.colors.text, accent, 0.22f)
			                                          : active_context()->style.colors.text;
			const Vec4 right_color = options.selected ? accent : active_context()->style.colors.text_muted;
			if (has_icon)
			{
				draw->AddText(ImVec2(start.x, start.y + (header_h - ImGui::GetTextLineHeight()) * 0.5f),
				              col_u32(options.selected ? accent : active_context()->style.colors.text_muted), options.icon);
			}
			if (has_title)
			{
				draw->AddText(ImVec2(title_x, start.y), col_u32(title_color), visible_label(title));
			}
			if (has_subtitle)
			{
				draw->AddText(ImVec2(title_x, start.y + title_h + 2.0f), col_u32(active_context()->style.colors.text_muted),
				              options.subtitle);
			}
			if (has_right)
			{
				draw->AddText(ImVec2(right_x, start.y + (header_h - ImGui::GetTextLineHeight()) * 0.5f), col_u32(right_color),
				              options.right_text);
			}
			ImGui::Dummy(ImVec2(std::max(1.0f, content_w - (has_right ? right_w + right_gap : 0.0f)), header_h));
			ImGui::Dummy(ImVec2(0.0f, spacing * 0.35f));
		}
		return true;
	}

	void end_card()
	{
		trinex_assert(!g_card_stack.empty() && "UI::end_card() called without matching begin_card()");
		if (g_card_stack.empty())
		{
			return;
		}

		const CardContext context = g_card_stack.back();
		g_card_stack.pop_back();

		const ImVec2 min   = ImGui::GetWindowPos();
		const ImVec2 size  = ImGui::GetWindowSize();
		const ImVec2 max   = add(min, size);
		const bool hovered = context.hoverable && !context.disabled && ImGui::IsMouseHoveringRect(min, max, true);
		AnimState& anim    = state_for(context.id);
		anim.hover         = approach(anim.hover, hovered ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.selected      = approach(anim.selected, context.selected ? 1.0f : 0.0f, active_context()->style.animation_speed);

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

	bool card_button(const char* title, const CardOptions& options)
	{
		cleanup_states();
		if (title != nullptr && title[0] != '\0')
		{
			ImGui::PushID(title);
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
		const bool has_title    = title != nullptr && visible_label(title)[0] != '\0';
		const bool has_subtitle = options.subtitle != nullptr && options.subtitle[0] != '\0';
		const bool has_icon     = options.icon != nullptr && options.icon[0] != '\0';
		const bool has_right    = options.right_text != nullptr && options.right_text[0] != '\0';
		const float icon_w      = has_icon ? ImGui::CalcTextSize(options.icon).x : 0.0f;
		const float right_w     = has_right ? ImGui::CalcTextSize(options.right_text).x : 0.0f;
		const float title_h     = has_title ? ImGui::GetTextLineHeight() : 0.0f;
		const float subtitle_h  = has_subtitle ? ImGui::GetTextLineHeight() : 0.0f;
		const float header_h    = std::max(
                std::max(title_h + (has_subtitle ? subtitle_h + 2.0f : 0.0f), has_icon ? ImGui::GetTextLineHeight() : 0.0f),
                has_right ? ImGui::GetTextLineHeight() : 0.0f);

		Vec2 size = options.size;
		if (size.x <= 0.0f)
		{
			size.x = ImGui::GetContentRegionAvail().x;
		}
		if (size.y <= 0.0f)
		{
			size.y = padding * 2.0f + std::max(header_h, active_context()->style.frame_height);
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

		const bool hovered = options.hoverable && !options.disabled && ImGui::IsItemHovered();
		const bool active  = !options.disabled && ImGui::IsItemActive();
		const bool clicked = !options.disabled && ImGui::IsItemClicked();
		anim.hover         = approach(anim.hover, hovered ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.active        = approach(anim.active, active ? 1.0f : 0.0f, active_context()->style.animation_speed * 1.5f);
		anim.selected      = approach(anim.selected, options.selected ? 1.0f : 0.0f, active_context()->style.animation_speed);

		const InteractiveRect rect = make_interactive_rect(pos, to_imvec(size), anim.hover, anim.active);

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
			Shadow shadow = scaled_shadow(current_shadow(), options.elevation);
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
		if (options.border)
		{
			const float alpha = options.selected ? 1.0f : Math::lerp(0.85f, 1.0f, anim.hover * 0.65f);
			draw->AddRect(rect.min, rect.max, col_u32(border, alpha * (options.disabled ? 0.7f : 1.0f)),
			              rounding * rect.rounding_scale, 0, active_context()->style.border_size);
		}

		const float icon_gap   = has_icon ? spacing * 0.75f : 0.0f;
		const float title_x    = rect.min.x + padding + icon_w + icon_gap;
		const float right_x    = rect.max.x - padding - right_w;
		const float header_top = rect.min.y + std::max(0.0f, (rect.visual_size.y - header_h) * 0.5f);
		const Vec4 title_color = options.selected ? Math::lerp(active_context()->style.colors.text, accent, 0.22f)
		                                          : active_context()->style.colors.text;
		const Vec4 right_color = options.selected ? accent : active_context()->style.colors.text_muted;
		const float alpha_mul  = options.disabled ? 0.55f : 1.0f;
		if (has_icon)
		{
			draw->AddText(ImVec2(rect.min.x + padding, header_top + (header_h - ImGui::GetTextLineHeight()) * 0.5f),
			              col_u32(options.selected ? accent : active_context()->style.colors.text_muted, alpha_mul),
			              options.icon);
		}
		if (has_title)
		{
			draw->AddText(ImVec2(title_x, header_top), col_u32(title_color, alpha_mul), visible_label(title));
		}
		if (has_subtitle)
		{
			draw->AddText(ImVec2(title_x, header_top + title_h + 2.0f),
			              col_u32(active_context()->style.colors.text_muted, alpha_mul), options.subtitle);
		}
		if (has_right)
		{
			draw->AddText(ImVec2(right_x, header_top + (header_h - ImGui::GetTextLineHeight()) * 0.5f),
			              col_u32(right_color, alpha_mul), options.right_text);
		}

		ImGui::PopID();
		return clicked;
	}

	/////////////////////// LAYOUT AND SCROLLING ///////////////////////

	bool begin_horizontal(const char* id_text, const Vec2& size, float align)
	{
		ImGui::BeginHorizontal(id_text, to_imvec(size), align);
		return true;
	}

	bool begin_horizontal(const void* id, const Vec2& size, float align)
	{
		ImGui::BeginHorizontal(id, to_imvec(size), align);
		return true;
	}

	bool begin_horizontal(int id, const Vec2& size, float align)
	{
		ImGui::BeginHorizontal(id, to_imvec(size), align);
		return true;
	}

	void end_horizontal()
	{
		ImGui::EndHorizontal();
	}

	bool begin_vertical(const char* id_text, const Vec2& size, float align)
	{
		ImGui::BeginVertical(id_text, to_imvec(size), align);
		return true;
	}

	bool begin_vertical(const void* id, const Vec2& size, float align)
	{
		ImGui::BeginVertical(id, to_imvec(size), align);
		return true;
	}

	bool begin_vertical(int id, const Vec2& size, float align)
	{
		ImGui::BeginVertical(id, to_imvec(size), align);
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

	bool begin_disabled(bool disabled)
	{
		ImGui::BeginDisabled(disabled);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * (disabled ? 0.55f : 1.0f));
		g_disabled_alpha_stack.push_back(active_context()->draw_alpha);
		if (disabled)
		{
			active_context()->draw_alpha *= 0.55f;
		}
		return true;
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

		const float eased          = apply_ease(anim.open, Ease::InOutQuad);
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

		const float visible_height   = std::max(anim.extra, measured_height) * apply_ease(anim.open, Ease::InOutQuad);
		active_context()->draw_alpha = context.previous_draw_alpha;
		ImGui::PopStyleVar();
		ImGui::PopClipRect();
		ImGui::SetCursorScreenPos(ImVec2(context.content_start.x, context.content_start.y + visible_height));
		ImGui::Dummy(ImVec2(0.0f, 0.0f));
		ImGui::PopID();
	}

	bool begin_scroll_area(const char* id, const Vec2& size, bool border, WindowFlags flags)
	{
		bool visible = ImGui::BeginChild(id, to_imvec(size), border, to_imgui_window_flags(flags));

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
		return ImGui::IsMouseDown(to_imgui_MouseButton(button));
	}

	bool is_mouse_clicked(MouseButton button)
	{
		return ImGui::IsMouseClicked(to_imgui_MouseButton(button));
	}

	bool is_mouse_released(MouseButton button)
	{
		return ImGui::IsMouseReleased(to_imgui_MouseButton(button));
	}

	bool is_mouse_double_clicked(MouseButton button)
	{
		return ImGui::IsMouseDoubleClicked(to_imgui_MouseButton(button));
	}

	bool is_mouse_dragging(MouseButton button, float lock_threshold)
	{
		return ImGui::IsMouseDragging(to_imgui_MouseButton(button), lock_threshold);
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
		return to_vec(ImGui::GetMouseDragDelta(to_imgui_MouseButton(button), lock_threshold));
	}

	void reset_mouse_drag_delta(MouseButton button)
	{
		ImGui::ResetMouseDragDelta(to_imgui_MouseButton(button));
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

	/////////////////////// TEXT AND TOOLTIPS ///////////////////////

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

	/////////////////////// IMAGES AND CONTROLS ///////////////////////

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

		if ((options.background || options.border) && has_shadow_override())
		{
			draw_shadow_rect(draw, pos, max, rounding, current_shadow());
		}

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
		frame_bg          = Math::lerp(frame_bg, active_context()->style.colors.background_hovered, anim.hover * 0.55f);
		frame_bg          = Math::lerp(frame_bg, active_context()->style.colors.background_active, anim.active * 0.65f);
		frame_border      = Math::lerp(frame_border, accent, anim.hover * 0.7f + anim.active * 0.3f);

		ImDrawList* draw                 = ImGui::GetWindowDrawList();
		const InteractiveRect frame_rect = make_interactive_rect(pos, frame_size, anim.hover, anim.active);
		const InteractiveRect image_rect =
		        make_interactive_rect(ImVec2(pos.x + padding, pos.y + padding), image_size, anim.hover, anim.active);
		const float image_rounding = std::max(0.0f, (rounding - padding * 0.5f) * image_rect.rounding_scale);

		if (has_shadow_override())
		{
			draw_shadow_rect(draw, frame_rect.min, frame_rect.max, rounding * frame_rect.rounding_scale, current_shadow());
		}

		if (options.background)
		{
			draw->AddRectFilled(frame_rect.min, frame_rect.max, col_u32(frame_bg), rounding * frame_rect.rounding_scale);
		}

		draw->AddImageRounded(ImTextureID(texture, sampler), image_rect.min, image_rect.max, to_imvec(options.uv0),
		                      to_imvec(options.uv1), col_u32(options.tint), image_rounding);

		if (options.border)
		{
			draw->AddRect(frame_rect.min, frame_rect.max, col_u32(frame_border), rounding * frame_rect.rounding_scale, 0,
			              active_context()->style.border_size);
		}

		ImGui::PopID();
		return clicked;
	}

	bool button(const char* label, const ButtonOptions& options)
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

		const Vec4 accent          = has_color(options.accent) ? options.accent : active_context()->style.colors.accent;
		Vec4 bg                    = options.ghost ? Vec4(0, 0, 0, 0) : active_context()->style.colors.background_active;
		bg                         = Math::lerp(bg, active_context()->style.colors.background_hovered, anim.hover);
		bg                         = Math::lerp(bg, accent, anim.active * 0.65f);
		ImDrawList* draw           = ImGui::GetWindowDrawList();
		const InteractiveRect rect = make_interactive_rect(pos, size, anim.hover, anim.active);
		if (has_shadow_override())
		{
			Shadow shadow = current_shadow();
			if (options.disabled)
			{
				shadow.color.w *= 0.55f;
			}
			draw_shadow_rect(draw, rect.min, rect.max, active_context()->style.rounding * rect.rounding_scale, shadow);
		}
		if (!options.ghost || anim.hover > 0.01f || anim.active > 0.01f)
		{
			draw->AddRectFilled(rect.min, rect.max, col_u32(bg, options.disabled ? 0.45f : 1.0f),
			                    active_context()->style.rounding * rect.rounding_scale);
		}
		draw->AddRect(
		        rect.min, rect.max,
		        col_u32(Math::lerp(active_context()->style.colors.border, accent, anim.hover), options.disabled ? 0.45f : 1.0f),
		        active_context()->style.rounding * rect.rounding_scale, 0, active_context()->style.border_size);

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
		draw->AddText(ImVec2(rect.center.x - ts.x * 0.5f, rect.center.y - ts.y * 0.5f), col_u32(tc), full_label.c_str());
		ImGui::PopID();
		return clicked;
	}

	bool icon_button(const char* icon, const char* label, const ButtonOptions& options)
	{
		ButtonOptions copy = options;
		copy.icon          = icon;
		return button(label, copy);
	}

	bool small_button(const char* label)
	{
		ButtonOptions options;
		options.size = Vec2(0.0f, 24.0f);
		return button(label, options);
	}

	bool ghost_button(const char* label, const Vec2& size)
	{
		ButtonOptions options;
		options.size  = size;
		options.ghost = true;
		return button(label, options);
	}

	bool danger_button(const char* label, const Vec2& size)
	{
		ButtonOptions options;
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
			const float s  = apply_ease(anim.value, Ease::OutBack);
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
		const float toggle_t = apply_ease(anim.value, Ease::InOutQuad);
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

	bool slider(const char* label, float* value, float min, float max, const char* format)
	{
		cleanup_states();
		ImGui::PushID(label);
		const ImGuiID id        = ImGui::GetID("slider");
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

	bool slider(const char* label, int* value, int min, int max, const char* format)
	{
		float v = value != nullptr ? static_cast<float>(*value) : 0.0f;
		(void) format;
		const bool changed = slider(label, &v, static_cast<float>(min), static_cast<float>(max), "%.0f");
		if (changed && value != nullptr)
		{
			*value = static_cast<int>(std::round(v));
		}
		return changed;
	}

	bool drag(const char* label, float* value, float speed, float min, float max, const char* format)
	{
		cleanup_states();
		ImGui::PushID(label);
		AnimState& anim = state_for(ImGui::GetID("drag"));
		push_input_frame_styles(anim.focus);
		const bool changed = ImGui::DragFloat(label, value, speed, min, max, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool drag(const char* label, int* value, float speed, int min, int max, const char* format)
	{
		cleanup_states();
		ImGui::PushID(label);
		AnimState& anim = state_for(ImGui::GetID("drag"));
		push_input_frame_styles(anim.focus);
		const bool changed = ImGui::DragInt(label, value, speed, min, max, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool drag(const char* label, Vec2* value, float speed, float min, float max, const char* format)
	{
		if (value == nullptr)
		{
			return false;
		}

		cleanup_states();
		ImGui::PushID(label);
		AnimState& anim = state_for(ImGui::GetID("drag"));
		push_input_frame_styles(anim.focus);
		const bool changed = ImGui::DragFloat2(label, &value->x, speed, min, max, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool drag(const char* label, Vec3* value, float speed, float min, float max, const char* format)
	{
		if (value == nullptr)
		{
			return false;
		}

		cleanup_states();
		ImGui::PushID(label);
		AnimState& anim = state_for(ImGui::GetID("drag"));
		push_input_frame_styles(anim.focus);
		const bool changed = ImGui::DragFloat3(label, &value->x, speed, min, max, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool drag(const char* label, Vec4* value, float speed, float min, float max, const char* format)
	{
		if (value == nullptr)
		{
			return false;
		}

		cleanup_states();
		ImGui::PushID(label);
		AnimState& anim = state_for(ImGui::GetID("drag"));
		push_input_frame_styles(anim.focus);
		const bool changed = ImGui::DragFloat4(label, &value->x, speed, min, max, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(const char* label, double* value, const char* format)
	{
		cleanup_states();
		ImGui::PushID(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		const bool changed = ImGui::InputDouble(label, value, 0.0, 0.0, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(const char* label, float* value, const char* format)
	{
		cleanup_states();
		ImGui::PushID(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		const bool changed = ImGui::InputFloat(label, value, 0.0f, 0.0f, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(const char* label, int* value)
	{
		cleanup_states();
		ImGui::PushID(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		const bool changed = ImGui::InputInt(label, value);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(const char* label, Vec2* value, const char* format)
	{
		if (value == nullptr)
		{
			return false;
		}

		cleanup_states();
		ImGui::PushID(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		const bool changed = ImGui::InputFloat2(label, &value->x, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(const char* label, Vec3* value, const char* format)
	{
		if (value == nullptr)
		{
			return false;
		}

		cleanup_states();
		ImGui::PushID(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		const bool changed = ImGui::InputFloat3(label, &value->x, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(const char* label, Vec4* value, const char* format)
	{
		if (value == nullptr)
		{
			return false;
		}

		cleanup_states();
		ImGui::PushID(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		const bool changed = ImGui::InputFloat4(label, &value->x, format);
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(const char* label, char* buffer, size_t buffer_size, InputTextFlags flags)
	{
		cleanup_states();
		ImGui::PushID(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		const bool changed = ImGui::InputText(label, buffer, buffer_size, to_imgui_input_text_flags(flags));
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(const char* label, const char* hint, char* buffer, size_t buffer_size, InputTextFlags flags)
	{
		cleanup_states();
		ImGui::PushID(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		const bool changed = ImGui::InputTextWithHint(label, hint, buffer, buffer_size, to_imgui_input_text_flags(flags));
		anim.hover         = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.focus         = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool input(const char* label, char* buffer, size_t buffer_size, const Vec2& size, InputTextFlags flags)
	{
		cleanup_states();
		ImGui::PushID(label);
		AnimState& anim = state_for(ImGui::GetID("input"));
		push_input_frame_styles(anim.focus);
		const bool changed =
		        ImGui::InputTextMultiline(label, buffer, buffer_size, to_imvec(size), to_imgui_input_text_flags(flags));
		anim.hover = approach(anim.hover, ImGui::IsItemHovered() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		anim.focus = approach(anim.focus, ImGui::IsItemActive() ? 1.0f : 0.0f, active_context()->style.animation_speed);
		pop_input_frame_styles();
		ImGui::PopID();
		return changed;
	}

	bool search_input(const char* label, char* buffer, size_t buffer_size)
	{
		return input(label, buffer, buffer_size);
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
		const float open_t = apply_ease(anim.open, Ease::InOutQuad);

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
		TreeNodeOptions options;
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
		draw->AddCircleFilled(center, diameter * 0.31f * apply_ease(anim.selected, Ease::OutBack),
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

	bool color_edit(const char* label, Vec4* color, bool alpha, ColorEditFlags flags)
	{
		if (color == nullptr)
		{
			return false;
		}
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, active_context()->style.rounding);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, to_imvec(active_context()->style.colors.background));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, to_imvec(active_context()->style.colors.background_hovered));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, to_imvec(active_context()->style.colors.background_active));
		const bool changed = alpha ? ImGui::ColorEdit4(label, &color->x, to_imgui_color_edit_flags(flags))
		                           : ImGui::ColorEdit3(label, &color->x, to_imgui_color_edit_flags(flags));
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		return changed;
	}

	bool keybind_input(const char* label, Keybind* binding)
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
				*binding                          = Keybind{};
				active_context()->keybind_capture = 0;
				changed                           = true;
			}
			else
			{
				for (int key_i = static_cast<int>(ui::Key::NamedKeyBegin); key_i < static_cast<int>(ui::Key::NamedKeyEnd);
				     ++key_i)
				{
					const ui::Key key = Key(static_cast<Key::Enum>(key_i));
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

	String keybind_to_string(const Keybind& binding)
	{
		if (binding.key_code == ui::Key::Undefined)
		{
			return "Undefined";
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

	bool is_keybind_pressed(const Keybind& binding, bool repeat)
	{
		return binding.key_code != ui::Key::Undefined && keybind_mods_match(binding) &&
		       ImGui::IsKeyPressed(to_imgui_key(binding.key_code), repeat);
	}

	/////////////////////// HEADERS, TREES AND NAVIGATION ///////////////////////

	bool begin_collapsing_header(const char* label, const HeaderOptions& options)
	{
		cleanup_states();
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
		const float indent   = g_tree_indent_stack.empty() ? 0.0f : g_tree_indent_stack.back();
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

	void open_command_palette()
	{
		g_command_palette.open                    = true;
		g_command_palette.focus_search_next_frame = true;
		g_command_palette.search[0]               = '\0';
		g_command_palette.selected_index          = 0;
		refresh_command_palette_results();
	}

	bool command_palette()
	{
		trinex_assert(active_context() && "UI::command_palette() requires an active UI context");
		AnimState& palette_anim      = state_for(ImGui::GetID("##command_palette_popup_anim"));
		AnimState& palette_blur_anim = state_for(ImGui::GetID("##command_palette_blur_anim"));
		palette_anim.open =
		        approach(palette_anim.open, g_command_palette.open ? 1.0f : 0.0f, active_context()->style.animation_speed);
		palette_blur_anim.open = approach(palette_blur_anim.open, g_command_palette.open ? 1.0f : 0.0f,
		                                  active_context()->style.animation_speed * 0.45f);
		if (g_command_palette.open && palette_anim.open > 0.995f)
		{
			palette_anim.open = 1.0f;
		}
		else if (!g_command_palette.open && palette_anim.open < 0.005f)
		{
			palette_anim.open = 0.0f;
		}
		if (g_command_palette.open && palette_blur_anim.open > 0.995f)
		{
			palette_blur_anim.open = 1.0f;
		}
		else if (!g_command_palette.open && palette_blur_anim.open < 0.005f)
		{
			palette_blur_anim.open = 0.0f;
		}

		if (!g_command_palette.open && palette_anim.open <= 0.0f && palette_blur_anim.open <= 0.0f)
		{
			return false;
		}
		const float eased_open         = apply_ease(palette_anim.open, Ease::InOutQuad);
		const float eased_blur         = apply_ease(palette_blur_anim.open, Ease::InOutQuad);
		const float popup_visual_alpha = g_command_palette.open ? eased_open : eased_blur;

		refresh_command_palette_results();

		bool execute_requested       = false;
		int execute_registry_index   = -1;
		bool ensure_selected_visible = g_command_palette.focus_search_next_frame;
		const bool has_results       = !g_command_palette.filtered_indices.empty();
		const int page_step          = 8;
		bool keyboard_navigation     = false;

		if (ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			g_command_palette.open = false;
		}
		if (has_results && ImGui::IsKeyPressed(ImGuiKey_DownArrow))
		{
			g_command_palette.selected_index = Math::clamp(g_command_palette.selected_index + 1, 0,
			                                               static_cast<int>(g_command_palette.filtered_indices.size()) - 1);
			ensure_selected_visible          = true;
			keyboard_navigation              = true;
		}
		if (has_results && ImGui::IsKeyPressed(ImGuiKey_UpArrow))
		{
			g_command_palette.selected_index = Math::clamp(g_command_palette.selected_index - 1, 0,
			                                               static_cast<int>(g_command_palette.filtered_indices.size()) - 1);
			ensure_selected_visible          = true;
			keyboard_navigation              = true;
		}
		if (has_results && ImGui::IsKeyPressed(ImGuiKey_PageDown))
		{
			g_command_palette.selected_index = Math::clamp(g_command_palette.selected_index + page_step, 0,
			                                               static_cast<int>(g_command_palette.filtered_indices.size()) - 1);
			ensure_selected_visible          = true;
			keyboard_navigation              = true;
		}
		if (has_results && ImGui::IsKeyPressed(ImGuiKey_PageUp))
		{
			g_command_palette.selected_index = Math::clamp(g_command_palette.selected_index - page_step, 0,
			                                               static_cast<int>(g_command_palette.filtered_indices.size()) - 1);
			ensure_selected_visible          = true;
			keyboard_navigation              = true;
		}
		if (has_results && ImGui::IsKeyPressed(ImGuiKey_Enter))
		{
			execute_requested      = true;
			execute_registry_index = g_command_palette.filtered_indices[g_command_palette.selected_index];
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
			g_command_palette.open = false;
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
		if (g_command_palette.focus_search_next_frame)
		{
			ImGui::SetKeyboardFocusHere();
			g_command_palette.focus_search_next_frame = false;
		}
		const bool search_submitted =
		        ImGui::InputTextWithHint("##command_palette_search", "Search commands...", g_command_palette.search,
		                                 sizeof(g_command_palette.search), ImGuiInputTextFlags_EnterReturnsTrue);
		pop_input_frame_styles();
		if (ImGui::IsItemEdited())
		{
			g_command_palette.selected_index = 0;
			refresh_command_palette_results();
		}

		if (!execute_requested && search_submitted && has_results)
		{
			execute_requested      = true;
			execute_registry_index = g_command_palette.filtered_indices[g_command_palette.selected_index];
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
			for (int visible_index = 0; visible_index < static_cast<int>(g_command_palette.filtered_indices.size());
			     ++visible_index)
			{
				const RegisteredCommand& command = g_command_palette.commands[g_command_palette.filtered_indices[visible_index]];
				const bool selected              = visible_index == g_command_palette.selected_index;
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
					g_command_palette.selected_index = visible_index;
				}
				if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && enabled)
				{
					execute_requested      = true;
					execute_registry_index = g_command_palette.filtered_indices[visible_index];
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

				if (visible_index + 1 < static_cast<int>(g_command_palette.filtered_indices.size()))
				{
					ImGui::Dummy(ImVec2(0.0f, spacing * 0.35f));
				}
			}
		}

		ImGui::EndChild();

		if (!g_command_palette.open && palette_anim.open <= 0.0f && palette_blur_anim.open <= 0.0f)
		{
			ImGui::CloseCurrentPopup();
		}

		end_glass_panel();
		pop_shadow();
		pop_blur();
		ImGui::PopStyleVar();
		active_context()->draw_alpha = previous_draw_alpha;
		ImGui::EndPopup();

		if (execute_requested && execute_registry_index >= 0 &&
		    execute_registry_index < static_cast<int>(g_command_palette.commands.size()))
		{
			Function<void()> action = g_command_palette.commands[execute_registry_index].action;
			if (action)
			{
				g_command_palette.open                    = false;
				g_command_palette.focus_search_next_frame = false;
				ImGui::CloseCurrentPopup();
				action();
				return true;
			}
		}
		return false;
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
		g_notifications.push_back(std::move(n));
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
