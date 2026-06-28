#pragma once
#include <IconsLucide.h>
#include <UI/theme.hpp>
#include <UI/types.hpp>

namespace Trinex::UI
{
	/////////////////////// LIFECYCLE AND FRAME ///////////////////////
	void initialize();
	void shutdown();
	Context* create_context(Trinex::Window* window);
	void destroy_context(Context* context);
	bool begin_frame(Context* context);
	void end_frame();

	/////////////////////// FONTS ///////////////////////
	void push_font(FontFamily family = FontFamily::Default, FontSize size = FontSize::Normal);
	void pop_font();
	void push_text_font(FontSize size = FontSize::Normal);
	void push_icons_font(FontSize size = FontSize::Normal);

	/////////////////////// UNITS ///////////////////////
	Unit px(float value);
	Unit dp(float value);
	Unit rem(float value);
	Unit percent(float value);
	Unit fill();
	Size size(Unit width, Unit height);
	Size px(float width, float height);
	Size dp(float width, float height);
	Size rem(float width, float height);
	Size percent(float width, float height);
	Size fill_size();
	float resolve(Unit value, Axis axis = Axis::X);
	Vec2 resolve(const Size& size);

	/////////////////////// STYLE AND EFFECTS ///////////////////////
	Style& style();
	void style(const Style& value);
	void push_style(const Style& value);
	void pop_style();
	void push_style_color(StyleColor color, const Vec4& value);
	void pop_style_color(u32 count = 1);
	void push_style_var(StyleVar var, f32 value);
	void push_style_var(StyleVar var, const Vec2& value);
	void pop_style_var(u32 count = 1);
	void paint(Vec2 pos, Size size, PaintFunction function, void* userdata = nullptr, usize userdata_size = 0,
	           DrawList draw_list = DrawList::Default);
	void paint(Size size, PaintFunction function, void* userdata = nullptr, usize userdata_size = 0,
	           DrawList draw_list = DrawList::Default);
	void paint(PaintFunction function, void* userdata = nullptr, usize userdata_size = 0, DrawList draw_list = DrawList::Default);
	void push_shadow(const Shadow& shadow);
	void pop_shadow();
	void push_blur(const BlurOptions& options);
	void pop_blur();
	void blur(const Vec2& min, const Vec2& max, DrawList draw_list, const BlurOptions& options);
	void push_render_scale(Vec2 scale, Vec2 pivot = Vec2(0.5f, 0.5f));
	void pop_render_scale();

	/////////////////////// ANIMATION AND IDENTITY ///////////////////////
	float apply_ease(float t, Ease mode = Ease::OutCubic);
	float animate_float(ID id, float target, float speed = -1.0f);
	Vec2 animate_vec2(ID id, const Vec2& target, float speed = -1.0f);
	Vec4 animate_color(ID id, const Vec4& target, float speed = -1.0f);
	void reset_animation(ID id);
	void clear_animations();
	void push_id(StringView value);
	void push_id(i32 value);
	void push_id(const void* value);
	void pop_id();
	ID id(StringView value);

	/////////////////////// DOCKING ///////////////////////
	bool is_window_docked();
	ID window_dock_id();
	void undock_window();

	bool begin_dockspace(const DockLayoutOptions& options);
	void end_docspace();
	bool begin_viewport_dockspace(const DockLayoutOptions& options = {});
	void end_viewport_dockspace();

	void dockspace(const DockLayoutOptions& options, const FunctionRef<void(DockLayout&)>& builder);
	void dockspace(const FunctionRef<void(DockLayout&)>& builder);
	void viewport_dockspace(const DockLayoutOptions& options, const FunctionRef<void(DockLayout&)>& builder);
	void viewport_dockspace(const FunctionRef<void(DockLayout&)>& builder);

	/////////////////////// WINDOWS AND CONTAINERS ///////////////////////
	bool begin_window(const char* name, bool* open = nullptr, const WindowOptions& options = {});
	void end_window();
	void register_widget(Context* context, Widget* widget);
	void unregister_widget(Context* context, Widget* widget);
	bool begin_panel(const char* id_text, const PanelOptions& options = {});
	void end_panel();
	bool begin_glass_panel(const char* id, Size size, const GlassOptions& options = {});
	void end_glass_panel();
	bool begin_group_panel(const char* label, const PanelOptions& options = {});
	void end_group_panel();
	bool begin_group();
	void end_group();
	bool begin_card(const char* title, const CardOptions& options = {});
	void end_card();
	bool card_button(const char* title, const CardOptions& options, const ActionRef& action);

	/////////////////////// LAYOUT AND SCROLLING ///////////////////////
	bool begin_horizontal(const char* id_text, Size size = Size(0, 0), float align = -1.0f);
	bool begin_horizontal(const void* id, Size size = Size(0, 0), float align = -1.0f);
	bool begin_horizontal(int id, Size size = Size(0, 0), float align = -1.0f);
	void end_horizontal();
	bool begin_vertical(const char* id_text, Size size = Size(0, 0), float align = -1.0f);
	bool begin_vertical(const void* id, Size size = Size(0, 0), float align = -1.0f);
	bool begin_vertical(int id, Size size = Size(0, 0), float align = -1.0f);
	void end_vertical();
	void spring(float weight = 1.0f, float spacing = -1.0f);
	void suspend_layout();
	void resume_layout();
	void separator();
	void spacing(Unit amount = Unit(-1.0f));
	void new_line();
	void dummy(const Size& size);
	void same_line(float offset_from_start_x = 0.0f, float spacing = -1.0f);
	void indent(Unit indent_w = Unit(0.0f));
	void unindent(Unit indent_w = Unit(0.0f));
	void align_text_to_frame_padding();
	bool begin_disabled(bool disabled = true);
	void end_disabled();
	bool begin_animated_area(const char* id_text, bool visible);
	void end_animated_area();
	bool begin_scroll_area(const char* id_text, Size size = Size(0, 0), bool border = false,
	                       WindowFlags flags = WindowFlags::Undefined);
	void end_scroll_area();
	void scroll_to_top();
	void scroll_to_bottom();

	/////////////////////// FRAME METRICS AND INPUT STATE ///////////////////////
	float delta_time();
	float frame_rate();
	double time_seconds();
	int frame_count();
	Vec2 viewport_pos();
	Vec2 viewport_size();
	Vec2 window_position();
	Vec2 window_size();
	float window_width();
	float window_height();
	float text_line_height();
	float text_line_height_with_spacing();
	float frame_height();
	float frame_height_with_spacing();
	Vec2 content_region_available();
	Vec2 cursor_position();
	void cursor_position(const Vec2& position);
	Vec2 cursor_screen_position();
	void cursor_screen_position(const Vec2& position);
	bool is_window_hovered();
	bool is_window_focused();
	bool is_window_appearing();
	Vec2 display_size();
	Vec2 framebuffer_scale();
	bool wants_keyboard_capture();
	bool wants_mouse_capture();
	bool wants_text_input();
	bool key_ctrl();
	bool key_shift();
	bool key_alt();
	bool key_super();
	bool is_key_down(Key key_code);
	bool is_key_pressed(Key key_code, bool repeat = true);
	bool is_key_released(Key key_code);
	bool is_mouse_pos_valid();
	bool is_mouse_down(MouseButton button = MouseButton::Left);
	bool is_mouse_clicked(MouseButton button = MouseButton::Left);
	bool is_mouse_released(MouseButton button = MouseButton::Left);
	bool is_mouse_double_clicked(MouseButton button = MouseButton::Left);
	bool is_mouse_dragging(MouseButton button = MouseButton::Left, float lock_threshold = -1.0f);
	Vec2 mouse_position();
	Vec2 mouse_delta();
	float mouse_wheel();
	float mouse_wheel_h();
	Vec2 mouse_drag_delta(MouseButton button = MouseButton::Left, float lock_threshold = -1.0f);
	void reset_mouse_drag_delta(MouseButton button = MouseButton::Left);
	bool is_mouse_hovering_rect(const Vec2& min, const Vec2& max, bool clip = true);
	bool is_any_item_hovered();
	bool is_any_item_active();
	bool is_any_item_focused();
	bool is_item_hovered();
	bool is_item_active();
	bool is_item_clicked();
	bool is_item_focused();
	bool is_item_edited();
	bool is_item_activated();
	bool is_item_deactivated();
	bool is_item_deactivated_after_edit();
	bool is_item_toggled_open();
	bool is_item_visible();
	bool is_mouse_hovering_item_rect();
	Vec2 item_rect_min();
	Vec2 item_rect_max();
	Vec2 item_rect_size();
	Vec2 item_rect_center();
	void keyboard_focus_here(i32 offset = 0);

	/////////////////////// TEXT AND TOOLTIPS ///////////////////////
	void text(const char* fmt, ...);
	void text_muted(const char* fmt, ...);
	void text_wrapped(const char* fmt, ...);
	void text_colored(const Vec4& color, const char* fmt, ...);
	void label(const char* text, const char* value = nullptr);
	void help_marker(const char* description);
	void tooltip(const char* text);
	void tooltip_delayed(const char* text, float delay = 0.45f);
	void tooltip_if_hovered(const char* text, float delay = 0.0f);
	void help_tooltip(const char* description);
	void clipboard_text(const char* text);

	/////////////////////// IMAGES AND CONTROLS ///////////////////////
	void image(const Texture& texture, Size size, const ImageOptions& options = {});
	bool image_button(const char* id_text, const Texture& texture, Size size, const ImageOptions& options = {});
	bool button(const char* label, const ButtonOptions& options = {});
	bool invisible_button(const char* label, const ButtonOptions& options = {});
	bool icon_button(const char* icon, const char* label, const ButtonOptions& options = {});
	bool small_button(const char* label);
	bool ghost_button(const char* label, Size size = Size(0, 0));
	bool danger_button(const char* label, Size size = Size(0, 0));
	bool checkbox(const char* label, bool* value);
	bool toggle(const char* label, bool* value);
	bool drag(const char* label, float* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
	          const char* format = "%.3f");
	bool drag(const char* label, int* value, float speed = 1.0f, int min = 0, int max = 0, const char* format = "%d");
	bool drag(const char* label, Vec2* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
	          const char* format = "%.3f");
	bool drag(const char* label, Vec3* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
	          const char* format = "%.3f");
	bool drag(const char* label, Vec4* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
	          const char* format = "%.3f");
	bool slider(const char* label, float* value, float min, float max, const char* format = "%.3f");
	bool slider(const char* label, int* value, int min, int max, const char* format = "%d");
	bool input(const char* label, double* value, const char* format = "%.6f");
	bool input(const char* label, float* value, const char* format = "%.3f");
	bool input(const char* label, int* value);
	bool input(const char* label, Vec2* value, const char* format = "%.3f");
	bool input(const char* label, Vec3* value, const char* format = "%.3f");
	bool input(const char* label, Vec4* value, const char* format = "%.3f");
	bool input(const char* label, char* buffer, size_t buffer_size, InputTextFlags flags = InputTextFlags::Undefined);
	bool input(const char* label, const char* hint, char* buffer, size_t buffer_size,
	           InputTextFlags flags = InputTextFlags::Undefined);
	bool input(const char* label, char* buffer, size_t buffer_size, Size size, InputTextFlags flags = InputTextFlags::Undefined);
	bool search_input(const char* label, char* buffer, size_t buffer_size);
	bool begin_combo(const char* label, const char* preview_value, ComboFlags flags = ComboFlags::Undefined);
	void end_combo();
	bool combo(const char* label, int* current_item, const char* const items[], int item_count);
	bool selectable(const char* label, bool selected = false, SelectableFlags flags = SelectableFlags::Undefined,
	                Size size = Size(0, 0));
	bool radio_button(const char* label, bool active);
	bool radio_button(const char* label, int* value, int button_value);
	bool segmented_control(const char* label, int* current_item, const char* const items[], int item_count,
	                       Size size = Size(0, 0));
	void progress_bar(float fraction, Size size = Size(-1, 0), const char* overlay = nullptr);
	void spinner(const char* id_text, Unit radius = Unit(8.0f), Unit thickness = Unit(2.0f),
	             const Vec4& color = Vec4(0, 0, 0, 0));
	bool color_edit(const char* label, Vec4* color, bool alpha = true, ColorEditFlags flags = ColorEditFlags::Undefined);
	bool keybind_input(const char* label, Keybind* binding);
	String keybind_to_string(const Keybind& binding);
	bool is_keybind_pressed(const Keybind& binding, bool repeat = false);

	/////////////////////// HEADERS, TREES AND NAVIGATION ///////////////////////
	bool begin_collapsing_header(const char* label, const HeaderOptions& options = {});
	void end_collapsing_header();
	bool begin_section_header(const char* label, const HeaderOptions& options = {});
	void end_section_header();
	bool section_header(const char* label, const HeaderOptions& options = {});
	bool tree_node(const char* label, const TreeNodeOptions& options = {});
	void tree_pop();
	bool tree_leaf(const char* label, const TreeNodeOptions& options = {});
	bool tree_item(const char* label, const TreeNodeOptions& options = {});
	bool selectable_tree_item(const char* label, bool selected = false, const TreeNodeOptions& options = {});

	bool begin_tab_bar(const char* id_text);
	void end_tab_bar();
	bool tab(const char* label, bool selected = false, Size size = Size(0, 0));
	bool sidebar_item(const char* label, bool selected = false, const char* icon = nullptr, const char* badge = nullptr);
	bool nav_item(const char* label, bool selected = false, const char* icon = nullptr);
	bool breadcrumb(const char* label, bool current = false);

	/////////////////////// POPUPS, MENUS AND COMMANDS ///////////////////////
	bool begin_modal(const char* name, bool* open = nullptr, WindowFlags flags = WindowFlags::AlwaysAutoResize);
	void open_modal(const char* name);
	void end_modal();
	bool begin_popup(const char* id_text, WindowFlags flags = WindowFlags::Undefined);
	void open_popup(const char* id_text);
	void end_popup();
	void close_popup();
	bool begin_context_menu(const char* id_text = nullptr);
	void end_context_menu();
	bool begin_menu_bar();
	void end_menu_bar();
	bool begin_main_menu_bar();
	void end_main_menu_bar();
	bool begin_menu(const char* label, bool enabled = true);
	void end_menu();
	bool menu_item(const char* label, const char* shortcut = nullptr, bool selected = false, bool enabled = true);
	bool menu_item(const char* label, const char* shortcut, bool* selected, bool enabled = true);
	void register_command(Context* context, const Command& command);
	void register_command(const Command& command);
	void register_console_commands(Context* context);
	void register_console_commands();
	void execute_command(StringView cmd);
	void open_command_palette();
	bool command_palette();

	/////////////////////// ICONS ///////////////////////
	void icon(const char* value, FontSize size = FontSize::Normal);
	void icon_colored(const Vec4& color, const char* value, FontSize size = FontSize::Normal);

	/////////////////////// FEEDBACK AND DATA VIEWS ///////////////////////
	void notification(const char* message, const NotificationOptions& options = {});
	ConfirmResult confirmation(const char* title, const char* message, const char* confirm_text = "Confirm",
	                           const char* cancel_text = "Cancel", bool danger = true);
	void badge(const char* text, const Vec4& color = Vec4(0, 0, 0, 0));
	void pill(const char* text, const Vec4& color = Vec4(0, 0, 0, 0));
	void status_dot(const Vec4& color, Unit radius = Unit(4.0f));
	void key_value_row(const char* key, const char* value);
	void property_row(const char* label, const Action& content, Unit label_width = Unit(140.0f));
	bool property_bool(const char* label, bool* value, bool use_checkbox = false, Unit label_width = Unit(140.0f));
	bool property_float(const char* label, float* value, float min, float max, const char* format = "%.3f",
	                    Unit label_width = Unit(140.0f));
	bool property_int(const char* label, int* value, int min, int max, const char* format = "%d",
	                  Unit label_width = Unit(140.0f));
	bool property_text(const char* label, char* buffer, size_t buffer_size, Unit label_width = Unit(140.0f));
	bool property_color(const char* label, Vec4* color, bool alpha = true, Unit label_width = Unit(140.0f));
	bool splitter(const char* id_text, float* size_a, float* size_b, Unit min_a = Unit(80.0f), Unit min_b = Unit(80.0f),
	              bool vertical = true, Unit thickness = Unit(4.0f));
	bool begin_toolbar(const char* id_text);
	void end_toolbar();
	bool begin_table(const char* id_text, int columns, TableFlags flags = TableFlags::Undefined, Size outer_size = Size(0, 0),
	                 Unit inner_width = Unit(0.0f));
	void end_table();
	void table_setup_column(const char* label, TableColumnFlags flags = TableColumnFlags::Undefined, float width_or_weight = 0.0f,
	                        ID id = ID(0));
	void table_headers();
	void table_next_row(TableRowFlags flags = TableRowFlags::Undefined, Unit min_row_height = Unit(0.0f));
	bool table_next_column();
	bool table_column(u32 idx);
	bool begin_list_box(const char* label, Size size = Size(0, 0));
	void end_list_box();
	bool list_item(const char* label, bool selected = false, const char* icon = nullptr, const char* badge = nullptr);
	bool filtered_list(const char* id_text, const char* filter, const char* const items[], int item_count, int* selected_index);
	bool begin_drag_source(DragDropFlags drag_drop = DragDropFlags::Undefined);
	bool drag_payload(const char* type, const void* data, size_t size, Condition condition = Condition::Undefined);
	bool drag_payload_text(const char* type, const char* text, Condition condition = Condition::Undefined);
	void end_drag_source();
	bool begin_drop_target();
	const DragDropPayload* accept_drop_payload(const char* type, DragDropFlags drag_drop = DragDropFlags::Undefined);
	void end_drop_target();
	void callout(const char* title, const char* message, NotificationKind kind = NotificationKind::Info);
	void banner(const char* title, const char* message, const Vec4& accent = Vec4(0, 0, 0, 0));
	bool hero(const HeroOptions& options);
	void empty_state(const StateOptions& options);
	void loading_state(const char* text = "Loading...");
	void error_state(const char* message, const char* title = "Error");

	/////////////////////// DRAW LISTS ///////////////////////
	DrawListHandle* draw_list(DrawList list = DrawList::Default);


	/////////////////////// INLINE STYLE AND EFFECTS HELPERS ///////////////////////

	inline void frame(Context* context, const ActionRef& func)
	{
		if (begin_frame(context))
		{
			func();
			end_frame();
		}
	}

	inline void style(const Style& value, const ActionRef& func)
	{
		push_style(value);
		func();
		pop_style();
	}

	inline void shadow(const Shadow& value, const ActionRef& func)
	{
		push_shadow(value);
		func();
		pop_shadow();
	}

	inline void blur(const BlurOptions& value, const ActionRef& func)
	{
		push_blur(value);
		func();
		pop_blur();
	}

	inline void blur(const Vec2& min, const Vec2& max, const BlurOptions& options)
	{
		blur(min, max, DrawList::Default, options);
	}

	/////////////////////// INLINE ANIMATION AND IDENTITY HELPERS ///////////////////////

	inline void id(const char* id_text, const ActionRef& func)
	{
		push_id(id_text);
		func();
		pop_id();
	}

	inline void id(int id_value, const ActionRef& func)
	{
		push_id(id_value);
		func();
		pop_id();
	}

	/////////////////////// INLINE DOCKING HELPERS ///////////////////////


	/////////////////////// INLINE WINDOWS AND CONTAINERS HELPERS ///////////////////////

	inline bool window(const char* name, bool* open, const WindowOptions& options, const ActionRef& func)
	{
		const bool visible = begin_window(name, open, options);
		if (visible)
		{
			func();
			end_window();
		}
		return visible;
	}

	inline bool window(const char* name, bool* open, const ActionRef& func)
	{
		return window(name, open, {}, func);
	}

	inline bool window(const char* name, const WindowOptions& options, const ActionRef& func)
	{
		return window(name, nullptr, options, func);
	}

	inline bool window(const char* name, const ActionRef& func)
	{
		return window(name, nullptr, {}, func);
	}

	inline bool panel(const char* id_text, const PanelOptions& options, const ActionRef& func)
	{
		const bool visible = begin_panel(id_text, options);
		if (visible)
		{
			func();
			end_panel();
		}
		return visible;
	}

	inline bool panel(const char* id_text, const ActionRef& func)
	{
		return panel(id_text, {}, func);
	}

	inline bool glass_panel(const char* id, Size size, const GlassOptions& options, const ActionRef& func)
	{
		const bool visible = begin_glass_panel(id, size, options);
		if (visible)
		{
			func();
			end_glass_panel();
		}
		return visible;
	}

	inline bool glass_panel(const char* id, Size size, const ActionRef& func)
	{
		return glass_panel(id, size, {}, func);
	}

	inline bool glass_panel(const char* id, const ActionRef& func)
	{
		return glass_panel(id, Size(0, 0), {}, func);
	}

	inline bool group_panel(const char* label, const PanelOptions& options, const ActionRef& func)
	{
		const bool visible = begin_group_panel(label, options);
		if (visible)
		{
			func();
			end_group_panel();
		}
		return visible;
	}

	inline bool group_panel(const char* label, const ActionRef& func)
	{
		return group_panel(label, {}, func);
	}

	inline bool group(const ActionRef& func)
	{
		const bool visible = begin_group();
		if (visible)
		{
			func();
			end_group();
		}
		return visible;
	}

	inline void card(const char* title, const CardOptions& options, const ActionRef& content)
	{
		if (begin_card(title, options))
		{
			content();
			end_card();
		}
	}

	inline bool card_button(const char* title, const ActionRef& action)
	{
		return card_button(title, {}, action);
	}

	/////////////////////// INLINE LAYOUT AND SCROLLING HELPERS ///////////////////////

	inline void horizontal(const char* id_text, Size size, float align, const ActionRef& func)
	{
		if (begin_horizontal(id_text, size, align))
		{
			func();
			end_horizontal();
		}
	}

	inline void horizontal(const char* id_text, Size size, const ActionRef& func)
	{
		horizontal(id_text, size, -1.0f, func);
	}

	inline void horizontal(const char* id_text, const ActionRef& func)
	{
		horizontal(id_text, Size(0, 0), -1.0f, func);
	}

	inline void horizontal(const void* id, Size size, float align, const ActionRef& func)
	{
		if (begin_horizontal(id, size, align))
		{
			func();
			end_horizontal();
		}
	}

	inline void horizontal(const void* id, Size size, const ActionRef& func)
	{
		horizontal(id, size, -1.0f, func);
	}

	inline void horizontal(const void* id, const ActionRef& func)
	{
		horizontal(id, Size(0, 0), -1.0f, func);
	}

	inline void horizontal(int id, Size size, float align, const ActionRef& func)
	{
		if (begin_horizontal(id, size, align))
		{
			func();
			end_horizontal();
		}
	}

	inline void horizontal(int id, Size size, const ActionRef& func)
	{
		horizontal(id, size, -1.0f, func);
	}

	inline void horizontal(int id, const ActionRef& func)
	{
		horizontal(id, Size(0, 0), -1.0f, func);
	}

	inline void vertical(const char* id_text, Size size, float align, const ActionRef& func)
	{
		if (begin_vertical(id_text, size, align))
		{
			func();
			end_vertical();
		}
	}

	inline void vertical(const char* id_text, Size size, const ActionRef& func)
	{
		vertical(id_text, size, -1.0f, func);
	}

	inline void vertical(const char* id_text, const ActionRef& func)
	{
		vertical(id_text, Size(0, 0), -1.0f, func);
	}

	inline void vertical(const void* id, Size size, float align, const ActionRef& func)
	{
		if (begin_vertical(id, size, align))
		{
			func();
			end_vertical();
		}
	}

	inline void vertical(const void* id, Size size, const ActionRef& func)
	{
		vertical(id, size, -1.0f, func);
	}

	inline void vertical(const void* id, const ActionRef& func)
	{
		vertical(id, Size(0, 0), -1.0f, func);
	}

	inline void vertical(int id, Size size, float align, const ActionRef& func)
	{
		if (begin_vertical(id, size, align))
		{
			func();
			end_vertical();
		}
	}

	inline void vertical(int id, Size size, const ActionRef& func)
	{
		vertical(id, size, -1.0f, func);
	}

	inline void vertical(int id, const ActionRef& func)
	{
		vertical(id, Size(0, 0), -1.0f, func);
	}

	inline void disabled(bool value, const ActionRef& func)
	{
		if (begin_disabled(value))
		{
			func();
			end_disabled();
		}
	}

	inline void disabled(const ActionRef& func)
	{
		disabled(true, func);
	}

	inline bool scroll_area(const char* id_text, Size size, bool border, WindowFlags flags, const ActionRef& func)
	{
		const bool visible = begin_scroll_area(id_text, size, border, flags);
		if (visible)
		{
			func();
			end_scroll_area();
		}
		return visible;
	}

	inline bool scroll_area(const char* id_text, Size size, bool border, const ActionRef& func)
	{
		return scroll_area(id_text, size, border, WindowFlags::Undefined, func);
	}

	inline bool scroll_area(const char* id_text, Size size, const ActionRef& func)
	{
		return scroll_area(id_text, size, false, WindowFlags::Undefined, func);
	}

	inline bool scroll_area(const char* id_text, const ActionRef& func)
	{
		return scroll_area(id_text, Size(0, 0), false, WindowFlags::Undefined, func);
	}

	inline void animated_area(const char* id_text, bool visible, const ActionRef& content)
	{
		if (begin_animated_area(id_text, visible))
		{
			content();
			end_animated_area();
		}
	}

	/////////////////////// INLINE IMAGES AND CONTROLS HELPERS ///////////////////////

	inline bool combo(const char* label, const char* preview_value, ComboFlags flags, const ActionRef& func)
	{
		const bool visible = begin_combo(label, preview_value, flags);
		if (visible)
		{
			func();
			end_combo();
		}
		return visible;
	}

	inline bool combo(const char* label, const char* preview_value, const ActionRef& func)
	{
		return combo(label, preview_value, ComboFlags::Undefined, func);
	}

	/////////////////////// INLINE HEADERS, TREES AND NAVIGATION HELPERS ///////////////////////

	inline bool tab_bar(const char* id_text, const ActionRef& func)
	{
		const bool visible = begin_tab_bar(id_text);
		if (visible)
		{
			func();
			end_tab_bar();
		}
		return visible;
	}

	inline void collapsing_header(const char* label, const HeaderOptions& options, const ActionRef& content)
	{
		if (begin_collapsing_header(label, options))
		{
			content();
			end_collapsing_header();
		}
	}

	inline void collapsing_header(const char* label, const ActionRef& content)
	{
		collapsing_header(label, {}, content);
	}

	inline void section_header(const char* label, const HeaderOptions& options, const ActionRef& content)
	{
		if (begin_section_header(label, options))
		{
			content();
			end_section_header();
		}
	}

	inline void section_header(const char* label, const ActionRef& content)
	{
		section_header(label, {}, content);
	}

	/////////////////////// INLINE POPUPS, MENUS AND COMMANDS HELPERS ///////////////////////

	inline bool modal(const char* name, bool* open, WindowFlags flags, const ActionRef& func)
	{
		const bool visible = begin_modal(name, open, flags);
		if (visible)
		{
			func();
			end_modal();
		}
		return visible;
	}

	inline bool modal(const char* name, bool* open, const ActionRef& func)
	{
		return modal(name, open, WindowFlags::AlwaysAutoResize, func);
	}

	inline bool modal(const char* name, const ActionRef& func)
	{
		return modal(name, nullptr, WindowFlags::AlwaysAutoResize, func);
	}

	inline bool popup(const char* id_text, WindowFlags flags, const ActionRef& func)
	{
		const bool visible = begin_popup(id_text, flags);
		if (visible)
		{
			func();
			end_popup();
		}
		return visible;
	}

	inline bool popup(const char* id_text, const ActionRef& func)
	{
		return popup(id_text, WindowFlags::Undefined, func);
	}

	inline bool context_menu(const char* id_text, const ActionRef& func)
	{
		const bool visible = begin_context_menu(id_text);
		if (visible)
		{
			func();
			end_context_menu();
		}
		return visible;
	}

	inline bool context_menu(const ActionRef& func)
	{
		return context_menu(nullptr, func);
	}

	inline bool menu_bar(const ActionRef& func)
	{
		const bool visible = begin_menu_bar();
		if (visible)
		{
			func();
			end_menu_bar();
		}
		return visible;
	}

	inline bool main_menu_bar(const ActionRef& func)
	{
		const bool visible = begin_main_menu_bar();
		if (visible)
		{
			func();
			end_main_menu_bar();
		}
		return visible;
	}

	inline bool menu(const char* label, bool enabled, const ActionRef& func)
	{
		const bool visible = begin_menu(label, enabled);
		if (visible)
		{
			func();
			end_menu();
		}
		return visible;
	}

	inline bool menu(const char* label, const ActionRef& func)
	{
		return menu(label, true, func);
	}

	/////////////////////// INLINE FEEDBACK AND DATA VIEWS HELPERS ///////////////////////

	inline bool toolbar(const char* id_text, const ActionRef& func)
	{
		const bool visible = begin_toolbar(id_text);
		if (visible)
		{
			func();
			end_toolbar();
		}
		return visible;
	}

	inline bool table(const char* id_text, int columns, TableFlags flags, Size outer_size, Unit inner_width,
	                  const ActionRef& func)
	{
		const bool visible = begin_table(id_text, columns, flags, outer_size, inner_width);
		if (visible)
		{
			func();
			end_table();
		}
		return visible;
	}

	inline bool table(const char* id_text, int columns, TableFlags flags, Size outer_size, const ActionRef& func)
	{
		return table(id_text, columns, flags, outer_size, 0.0f, func);
	}

	inline bool table(const char* id_text, int columns, TableFlags flags, const ActionRef& func)
	{
		return table(id_text, columns, flags, Size(0, 0), Unit(0.0f), func);
	}

	inline bool table(const char* id_text, int columns, const ActionRef& func)
	{
		return table(id_text, columns, TableFlags::Undefined, Size(0, 0), Unit(0.0f), func);
	}

	inline bool list_box(const char* label, Size size, const ActionRef& func)
	{
		const bool visible = begin_list_box(label, size);
		if (visible)
		{
			func();
			end_list_box();
		}
		return visible;
	}

	inline bool list_box(const char* label, const ActionRef& func)
	{
		return list_box(label, Size(0, 0), func);
	}

	inline bool drag_source(DragDropFlags drag_drop, const ActionRef& func)
	{
		const bool visible = begin_drag_source(drag_drop);
		if (visible)
		{
			func();
			end_drag_source();
		}
		return visible;
	}

	inline bool drag_source(const ActionRef& func)
	{
		return drag_source(DragDropFlags::Undefined, func);
	}

	inline bool drop_target(const ActionRef& func)
	{
		const bool visible = begin_drop_target();
		if (visible)
		{
			func();
			end_drop_target();
		}
		return visible;
	}

	/////////////////////// INLINE PAINT HELPERS ///////////////////////
	template<typename F>
	    requires(TriviallyStored<F>)
	inline void paint(Vec2 pos, Size size, F&& f)
	{
		auto callback = +[](void* userdata) { (*static_cast<F*>(userdata))(); };
		paint(pos, size, callback, &f, sizeof(f));
	}

	template<typename F>
	    requires(TriviallyStored<F>)
	inline void paint(Vec2 pos, Size size, DrawList draw_list, F&& f)
	{
		auto callback = +[](void* userdata) { (*static_cast<F*>(userdata))(); };
		paint(pos, size, callback, &f, sizeof(f), draw_list);
	}

	template<typename F>
	    requires(TriviallyStored<F>)
	inline void paint(Size size, F&& f)
	{
		auto callback = +[](void* userdata) { (*static_cast<F*>(userdata))(); };
		paint(size, callback, &f, sizeof(f));
	}

	template<typename F>
	    requires(TriviallyStored<F>)
	inline void paint(Size size, DrawList draw_list, F&& f)
	{
		auto callback = +[](void* userdata) { (*static_cast<F*>(userdata))(); };
		paint(size, callback, &f, sizeof(f), draw_list);
	}

	template<typename F>
	    requires(TriviallyStored<F>)
	inline void paint(F&& f)
	{
		auto callback = +[](void* userdata) { (*static_cast<F*>(userdata))(); };
		paint(callback, &f, sizeof(f));
	}

	template<typename F>
	    requires(TriviallyStored<F>)
	inline void paint(DrawList draw_list, F&& f)
	{
		auto callback = +[](void* userdata) { (*static_cast<F*>(userdata))(); };
		paint(callback, &f, sizeof(f), draw_list);
	}
}// namespace Trinex::UI
