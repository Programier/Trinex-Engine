#pragma once
#include <Core/etl/function.hpp>
#include <Core/etl/string.hpp>
#include <Core/math/vector.hpp>

namespace Trinex
{
	class Window;
	class RHITexture;
	class RHISampler;
}// namespace Trinex

namespace Trinex::UI
{
	using Vec2 = Vector2f;
	using Vec3 = Vector3f;
	using Vec4 = Vector4f;

	class ID
	{
	public:
		using ValueType = i32;

	private:
		ValueType m_id;

	public:
		constexpr ID() : m_id(0) {}
		constexpr explicit ID(ValueType id) : m_id(id) {}

		constexpr ValueType value() const { return m_id; }

		constexpr explicit operator bool() const { return m_id != 0; }

		constexpr bool operator==(const ID& other) const { return m_id == other.m_id; }
		constexpr bool operator!=(const ID& other) const { return m_id != other.m_id; }
		constexpr bool operator<(const ID& other) const { return m_id < other.m_id; }
		constexpr bool operator>(const ID& other) const { return m_id > other.m_id; }
		constexpr bool operator<=(const ID& other) const { return m_id <= other.m_id; }
		constexpr bool operator>=(const ID& other) const { return m_id >= other.m_id; }
	};

	struct Ease {
		enum Enum : u8
		{
			Linear      = 0,
			InQuad      = 1,
			OutQuad     = 2,
			InOutQuad   = 3,
			OutCubic    = 4,
			OutBack     = 5,
			linear      = Linear,
			in_quad     = InQuad,
			out_quad    = OutQuad,
			in_out_quad = InOutQuad,
			out_cubic   = OutCubic,
			out_back    = OutBack
		};

		trinex_enum_struct(Ease);
	};

	struct NotificationKind {
		enum Enum : u8
		{
			Info,
			Success,
			Warning,
			Error,
			info    = Info,
			success = Success,
			warning = Warning,
			error   = Error
		};

		trinex_enum_struct(NotificationKind);
	};

	struct Key {
		enum Enum : u16
		{
			Undefined     = 0,
			NamedKeyBegin = 512,
			Tab           = 512,
			LeftArrow,
			RightArrow,
			UpArrow,
			DownArrow,
			PageUp,
			PageDown,
			Home,
			End,
			Insert,
			Del,
			Backspace,
			Space,
			Enter,
			Escape,
			LeftCtrl,
			LeftShift,
			LeftAlt,
			LeftSuper,
			RightCtrl,
			RightShift,
			RightAlt,
			RightSuper,
			Menu,
			Key0,
			Key1,
			Key2,
			Key3,
			Key4,
			Key5,
			Key6,
			Key7,
			Key8,
			Key9,
			A,
			B,
			C,
			D,
			E,
			F,
			G,
			H,
			I,
			J,
			K,
			L,
			M,
			N,
			O,
			P,
			Q,
			R,
			S,
			T,
			U,
			V,
			W,
			X,
			Y,
			Z,
			F1,
			F2,
			F3,
			F4,
			F5,
			F6,
			F7,
			F8,
			F9,
			F10,
			F11,
			F12,
			F13,
			F14,
			F15,
			F16,
			F17,
			F18,
			F19,
			F20,
			F21,
			F22,
			F23,
			F24,
			NamedKeyEnd,
			none            = Undefined,
			named_key_begin = NamedKeyBegin,
			left_ctrl       = LeftCtrl,
			left_shift      = LeftShift,
			left_alt        = LeftAlt,
			left_super      = LeftSuper,
			right_ctrl      = RightCtrl,
			right_shift     = RightShift,
			right_alt       = RightAlt,
			right_super     = RightSuper,
			named_key_end   = NamedKeyEnd
		};

		trinex_enum_struct(Key);
	};

	struct MouseButton {
		enum Enum : u8
		{
			Left   = 0,
			Right  = 1,
			Middle = 2,
			left   = Left,
			right  = Right,
			middle = Middle
		};

		trinex_enum_struct(MouseButton);
	};

	struct WindowFlags {
		enum Enum : u16
		{
			Undefined         = 0,
			NoTitleBar        = 1u << 0,
			NoResize          = 1u << 1,
			NoMove            = 1u << 2,
			NoScrollbar       = 1u << 3,
			NoScrollWithMouse = 1u << 4,
			NoCollapse        = 1u << 5,
			AlwaysAutoResize  = 1u << 6,
			NoBackground      = 1u << 7,
			NoSavedSettings   = 1u << 8,
			NoMouseInputs     = 1u << 9,
			MenuBar           = 1u << 10
		};

		trinex_bitfield_enum_struct(WindowFlags, u16);
	};

	struct InputTextFlags {
		enum Enum : u32
		{
			Undefined          = 0,
			CharsDecimal       = 1u << 0,
			CharsHexadecimal   = 1u << 1,
			CharsScientific    = 1u << 2,
			CharsUppercase     = 1u << 3,
			CharsNoBlank       = 1u << 4,
			AllowTabInput      = 1u << 5,
			EnterReturnsTrue   = 1u << 6,
			EscapeClearsAll    = 1u << 7,
			CtrlEnterNewLine   = 1u << 8,
			ReadOnly           = 1u << 9,
			Password           = 1u << 10,
			AlwaysOverwrite    = 1u << 11,
			AutoSelectAll      = 1u << 12,
			NoHorizontalScroll = 1u << 15,
			NoUndoRedo         = 1u << 16,
			ElideLeft          = 1u << 17
		};

		trinex_bitfield_enum_struct(InputTextFlags, u32);
	};

	struct ComboFlags {
		enum Enum : u8
		{
			Undefined       = 0,
			PopupAlignLeft  = 1u << 0,
			HeightSmall     = 1u << 1,
			HeightRegular   = 1u << 2,
			HeightLarge     = 1u << 3,
			HeightLargest   = 1u << 4,
			NoArrowButton   = 1u << 5,
			NoPreview       = 1u << 6,
			WidthFitPreview = 1u << 7
		};

		trinex_bitfield_enum_struct(ComboFlags, u8);
	};

	struct SelectableFlags {
		enum Enum : u8
		{
			Undefined         = 0,
			NoAutoClosePopups = 1u << 0,
			SpanAllColumns    = 1u << 1,
			AllowDoubleClick  = 1u << 2,
			Disabled          = 1u << 3,
			AllowOverlap      = 1u << 4,
			Highlight         = 1u << 5,
			SelectOnNav       = 1u << 6
		};

		trinex_bitfield_enum_struct(SelectableFlags, u8);
	};

	struct ColorEditFlags {
		enum Enum : u8
		{
			Undefined = 0,
		};

		trinex_bitfield_enum_struct(ColorEditFlags, u8);
	};

	struct Condition {
		enum Enum : u8
		{
			Undefined    = 0,
			Always       = 1u << 0,
			Once         = 1u << 1,
			FirstUseEver = 1u << 2,
			Appearing    = 1u << 3
		};

		trinex_bitfield_enum_struct(Condition, u8);
	};

	struct TableFlags {
		enum Enum : u16
		{
			Undefined         = 0,
			Resizable         = 1u << 0,
			Reorderable       = 1u << 1,
			Hideable          = 1u << 2,
			Sortable          = 1u << 3,
			NoSavedSettings   = 1u << 4,
			ContextMenuInBody = 1u << 5,
			RowBg             = 1u << 6,
			BordersInnerH     = 1u << 7,
			BordersOuterH     = 1u << 8,
			BordersInnerV     = 1u << 9,
			BordersOuterV     = 1u << 10,
			BordersH          = BordersInnerH | BordersOuterH,
			BordersV          = BordersInnerV | BordersOuterV,
			BordersInner      = BordersInnerV | BordersInnerH,
			BordersOuter      = BordersOuterV | BordersOuterH,
			Borders           = BordersInner | BordersOuter
		};

		trinex_bitfield_enum_struct(TableFlags, u16);
	};

	struct TableColumnFlags {
		enum Enum : u8
		{
			Undefined = 0
		};

		trinex_bitfield_enum_struct(TableColumnFlags, u8);
	};

	struct TableRowFlags {
		enum Enum : u8
		{
			Undefined = 0,
			Headers   = 1u << 0
		};

		trinex_bitfield_enum_struct(TableRowFlags, u8);
	};

	struct DragDropFlags {
		enum Enum : u16
		{
			Undefined               = 0,
			SourceNoPreviewTooltip  = 1u << 0,
			SourceNoDisableHover    = 1u << 1,
			SourceNoHoldOpen        = 1u << 2,
			SourceAllowNullId       = 1u << 3,
			SourceExtern            = 1u << 4,
			PayloadAutoExpire       = 1u << 5,
			PayloadNoCrossContext   = 1u << 6,
			PayloadNoCrossProcess   = 1u << 7,
			AcceptBeforeDelivery    = 1u << 10,
			AcceptNoDrawDefaultRect = 1u << 11,
			AcceptNoPreviewTooltip  = 1u << 12,
			AcceptDrawAsHovered     = 1u << 13,
			AcceptPeekOnly          = AcceptBeforeDelivery | AcceptNoDrawDefaultRect
		};

		trinex_bitfield_enum_struct(DragDropFlags, u16);
	};

	struct ConfirmResult {
		enum Enum : u8
		{
			Undefined = 0,
			Confirmed = 1,
			Canceled  = 2,
			none      = Undefined,
			confirmed = Confirmed,
			cancelled = Canceled
		};

		trinex_enum_struct(ConfirmResult);
	};

	struct DragDropPayload {
		const void* data = nullptr;
		int data_size    = 0;
		bool preview     = false;
		bool delivery    = false;
	};

	struct ColorTheme {
		Vec4 text               = Vec4(0.92f, 0.94f, 0.96f, 1.00f);
		Vec4 text_muted         = Vec4(0.58f, 0.63f, 0.70f, 1.00f);
		Vec4 text_disabled      = Vec4(0.38f, 0.42f, 0.48f, 1.00f);
		Vec4 background         = Vec4(0.08f, 0.10f, 0.13f, 1.00f);
		Vec4 background_hovered = Vec4(0.13f, 0.16f, 0.21f, 1.00f);
		Vec4 background_active  = Vec4(0.17f, 0.21f, 0.28f, 1.00f);
		Vec4 panel              = Vec4(0.10f, 0.12f, 0.16f, 1.00f);
		Vec4 border             = Vec4(0.24f, 0.29f, 0.36f, 1.00f);
		Vec4 accent             = Vec4(0.28f, 0.62f, 0.95f, 1.00f);
		Vec4 accent_hovered     = Vec4(0.36f, 0.70f, 1.00f, 1.00f);
		Vec4 accent_active      = Vec4(0.18f, 0.47f, 0.82f, 1.00f);
		Vec4 success            = Vec4(0.29f, 0.78f, 0.48f, 1.00f);
		Vec4 warning            = Vec4(0.95f, 0.68f, 0.22f, 1.00f);
		Vec4 error              = Vec4(0.95f, 0.32f, 0.32f, 1.00f);
	};

	struct Style {
		float animation_speed = 12.0f;
		float rounding        = 8.0f;
		float border_size     = 1.0f;
		float frame_height    = 32.0f;
		float padding         = 10.0f;
		float spacing         = 8.0f;
		float alpha           = 1.0f;
		Vec2 hover_scale      = Vec2(0.02f, 0.02f);
		ColorTheme colors;
	};

	struct PanelOptions {
		Vec2 size             = Vec2(0.0f, 0.0f);
		bool border           = true;
		bool background       = true;
		float rounding        = -1.0f;
		Vec4 background_color = Vec4(0, 0, 0, 0);
	};

	struct ImageOptions {
		RHISampler* sampler   = nullptr;
		Vec2 uv0              = Vec2(0.0f, 0.0f);
		Vec2 uv1              = Vec2(1.0f, 1.0f);
		Vec4 tint             = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		bool background       = true;
		bool border           = true;
		float padding         = 6.0f;
		float rounding        = -1.0f;
		Vec4 background_color = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
		Vec4 border_color     = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
		Vec4 accent           = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
	};

	struct ButtonOptions {
		Vec2 size        = Vec2(0.0f, 0.0f);
		const char* icon = nullptr;
		bool disabled    = false;
		bool ghost       = false;
		Vec4 accent      = Vec4(0, 0, 0, 0);
	};

	struct HeaderOptions {
		const char* icon       = nullptr;
		const char* right_text = nullptr;
		bool default_open      = false;
		bool full_row_click    = true;
		bool disabled          = false;
		bool* open             = nullptr;
		Vec4 accent            = Vec4(0, 0, 0, 0);
	};

	struct TreeNodeOptions {
		const char* icon    = nullptr;
		const char* badge   = nullptr;
		bool default_open   = false;
		bool selected       = false;
		bool leaf           = false;
		bool full_row_click = true;
		bool* open          = nullptr;
		Vec4 accent         = Vec4(0, 0, 0, 0);
	};

	struct NotificationOptions {
		NotificationKind kind    = NotificationKind::Info;
		float duration           = 3.0f;
		const char* title        = nullptr;
		const char* action_label = nullptr;
		Function<void()> action;
	};

	struct Keybind {
		Key key_code = Key::Undefined;
		bool ctrl    = false;
		bool shift   = false;
		bool alt     = false;
		bool super   = false;
	};

	struct StateOptions {
		const char* icon        = nullptr;
		const char* title       = nullptr;
		const char* description = nullptr;
	};

	using ease                 = Ease;
	using key                  = Key;
	using mouse_button         = MouseButton;
	using style                = Style;
	using color_theme          = ColorTheme;
	using panel_options        = PanelOptions;
	using button_options       = ButtonOptions;
	using header_options       = HeaderOptions;
	using tree_node_options    = TreeNodeOptions;
	using notification_kind    = NotificationKind;
	using notification_options = NotificationOptions;
	using keybind              = Keybind;
	using confirm_result       = ConfirmResult;
	using state_options        = StateOptions;

	struct DisabledScope {
		explicit DisabledScope(bool disabled = true);
		~DisabledScope();
		DisabledScope(const DisabledScope&)            = delete;
		DisabledScope& operator=(const DisabledScope&) = delete;
	};

	struct StyleScope {
		explicit StyleScope(const Style& value);
		~StyleScope();
		StyleScope(const StyleScope&)            = delete;
		StyleScope& operator=(const StyleScope&) = delete;
	};

	struct IdScope {
		explicit IdScope(const char* id);
		explicit IdScope(int id);
		~IdScope();
		IdScope(const IdScope&)            = delete;
		IdScope& operator=(const IdScope&) = delete;
	};

	struct Context;

	void initialize();
	void shutdown();
	Context* create_context(Trinex::Window* window);
	void destroy_context(Context* context);

	void begin_frame(Context* context);
	void end_frame();

	Style& get_style();
	void set_style(const Style& value);
	void push_style(const Style& value);
	void pop_style();

	float apply_ease(float t, Ease mode = Ease::OutCubic);
	float animate_float(ID id, float target, float speed = -1.0f);
	Vec2 animate_vec2(ID id, const Vec2& target, float speed = -1.0f);
	Vec4 animate_color(ID id, const Vec4& target, float speed = -1.0f);
	void reset_animation(ID id);
	void clear_animations();
	void push_id(const char* id_text);
	void push_id(int id_value);
	void pop_id();
	ID id(const char* id_text);

	bool begin_window(const char* name, bool* open = nullptr, WindowFlags flags = WindowFlags::Undefined);
	void end_window();
	void create_window(const char* name, const Function<void()>& content, WindowFlags flags = WindowFlags::Undefined);
	void create_window(const char* name, bool open, const Function<void()>& content, WindowFlags flags = WindowFlags::Undefined);
	bool is_window_open(const char* name);
	void open_window(const char* name);
	void close_window();
	void close_window(const char* name);
	bool begin_panel(const char* id_text, const PanelOptions& options = {});
	void end_panel();
	bool begin_child_panel(const char* id_text, const Vec2& size = Vec2(0, 0), const PanelOptions& options = {});
	void end_child_panel();
	bool begin_group_panel(const char* label, const Vec2& size = Vec2(0, 0), const PanelOptions& options = {});
	void end_group_panel();
	void separator();
	void spacing(float amount = -1.0f);
	void same_line(float offset_from_start_x = 0.0f, float spacing = -1.0f);
	void begin_disabled(bool disabled = true);
	void end_disabled();
	bool begin_animated_area(const char* id_text, bool visible);
	void end_animated_area();
	void animated_area(const char* id_text, bool visible, const Function<void()>& content);
	bool begin_scroll_area(const char* id_text, const Vec2& size = Vec2(0, 0), bool border = false,
	                       WindowFlags flags = WindowFlags::Undefined);
	void end_scroll_area();
	void scroll_to_top();
	void scroll_to_bottom();
	float delta_time();
	float frame_rate();
	double time_seconds();
	int frame_count();
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

	void text(const char* fmt, ...);
	void text_muted(const char* fmt, ...);
	void text_colored(const Vec4& color, const char* fmt, ...);
	void label(const char* text, const char* value = nullptr);
	void help_marker(const char* description);
	void tooltip(const char* text);
	void tooltip_delayed(const char* text, float delay = 0.45f);
	void tooltip_if_hovered(const char* text, float delay = 0.0f);
	void help_tooltip(const char* description);

	void image(RHITexture* texture, const Vec2& size, const ImageOptions& options = {});
	bool image_button(const char* id_text, RHITexture* texture, const Vec2& size, const ImageOptions& options = {});

	bool button(const char* label, const ButtonOptions& options = {});
	bool icon_button(const char* icon, const char* label, const ButtonOptions& options = {});
	bool small_button(const char* label);
	bool ghost_button(const char* label, const Vec2& size = Vec2(0, 0));
	bool danger_button(const char* label, const Vec2& size = Vec2(0, 0));
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
	bool input(const char* label, char* buffer, size_t buffer_size, const Vec2& size,
	           InputTextFlags flags = InputTextFlags::Undefined);
	bool search_input(const char* label, char* buffer, size_t buffer_size);
	bool begin_combo(const char* label, const char* preview_value, ComboFlags flags = ComboFlags::Undefined);
	void end_combo();
	bool combo(const char* label, int* current_item, const char* const items[], int item_count);
	bool selectable(const char* label, bool selected = false, SelectableFlags flags = SelectableFlags::Undefined,
	                const Vec2& size = Vec2(0, 0));
	bool radio_button(const char* label, bool active);
	bool radio_button(const char* label, int* value, int button_value);
	bool segmented_control(const char* label, int* current_item, const char* const items[], int item_count,
	                       const Vec2& size = Vec2(0, 0));
	void progress_bar(float fraction, const Vec2& size = Vec2(-1, 0), const char* overlay = nullptr);
	void spinner(const char* id_text, float radius = 8.0f, float thickness = 2.0f, const Vec4& color = Vec4(0, 0, 0, 0));
	bool color_edit(const char* label, Vec4* color, bool alpha = true, ColorEditFlags flags = ColorEditFlags::Undefined);
	bool keybind_input(const char* label, Keybind* binding);
	String keybind_to_string(const Keybind& binding);
	bool is_keybind_pressed(const Keybind& binding, bool repeat = false);

	bool begin_collapsing_header(const char* label, const HeaderOptions& options = {});
	void end_collapsing_header();
	bool collapsing_header(const char* label, const HeaderOptions& options = {});
	void collapsing_header(const char* label, const Function<void()>& content, const HeaderOptions& options = {});
	bool begin_section_header(const char* label, const HeaderOptions& options = {});
	void end_section_header();
	bool section_header(const char* label, const HeaderOptions& options = {});
	void section_header(const char* label, const Function<void()>& content, const HeaderOptions& options = {});
	bool tree_node(const char* label, const TreeNodeOptions& options = {});
	void tree_pop();
	bool tree_leaf(const char* label, const TreeNodeOptions& options = {});
	bool tree_item(const char* label, const TreeNodeOptions& options = {});
	bool selectable_tree_item(const char* label, bool selected = false, const TreeNodeOptions& options = {});

	bool begin_tab_bar(const char* id_text);
	void end_tab_bar();
	bool tab(const char* label, bool selected = false, const Vec2& size = Vec2(0, 0));
	bool sidebar_item(const char* label, bool selected = false, const char* icon = nullptr, const char* badge = nullptr);
	bool nav_item(const char* label, bool selected = false, const char* icon = nullptr);
	bool breadcrumb(const char* label, bool current = false);

	bool begin_modal(const char* name, bool* open = nullptr, WindowFlags flags = WindowFlags::AlwaysAutoResize);
	void open_modal(const char* name);
	void end_modal();
	bool begin_popup(const char* id_text, WindowFlags flags = WindowFlags::Undefined);
	void open_popup(const char* id_text);
	void end_popup();
	bool begin_context_menu(const char* id_text = nullptr);
	void end_context_menu();
	bool begin_menu_bar();
	void end_menu_bar();
	bool begin_menu(const char* label, bool enabled = true);
	void end_menu();
	bool menu_item(const char* label, const char* shortcut = nullptr, bool selected = false, bool enabled = true);
	bool menu_item(const char* label, const char* shortcut, bool* selected, bool enabled = true);

	void notification(const char* message, const NotificationOptions& options = {});
	ConfirmResult confirmation(const char* title, const char* message, const char* confirm_text = "Confirm",
	                           const char* cancel_text = "Cancel", bool danger = true);

	void badge(const char* text, const Vec4& color = Vec4(0, 0, 0, 0));
	void pill(const char* text, const Vec4& color = Vec4(0, 0, 0, 0));
	void status_dot(const Vec4& color, float radius = 4.0f);
	void key_value_row(const char* key, const char* value);
	void property_row(const char* label, const Function<void()>& content, float label_width = 140.0f);
	bool property_bool(const char* label, bool* value, bool use_checkbox = false, float label_width = 140.0f);
	bool property_float(const char* label, float* value, float min, float max, const char* format = "%.3f",
	                    float label_width = 140.0f);
	bool property_int(const char* label, int* value, int min, int max, const char* format = "%d", float label_width = 140.0f);
	bool property_text(const char* label, char* buffer, size_t buffer_size, float label_width = 140.0f);
	bool property_color(const char* label, Vec4* color, bool alpha = true, float label_width = 140.0f);
	bool splitter(const char* id_text, float* size_a, float* size_b, float min_a = 80.0f, float min_b = 80.0f,
	              bool vertical = true, float thickness = 4.0f);
	bool begin_toolbar(const char* id_text);
	void end_toolbar();
	bool begin_table(const char* id_text, int columns, TableFlags flags = TableFlags::Undefined,
	                 const Vec2& outer_size = Vec2(0, 0), float inner_width = 0.0f);
	void end_table();
	void table_column(const char* label, TableColumnFlags flags = TableColumnFlags::Undefined, float width_or_weight = 0.0f,
	                  ID id = ID(0));
	void table_headers();
	void table_next_row(TableRowFlags flags = TableRowFlags::Undefined, float min_row_height = 0.0f);
	bool table_next_column();
	bool begin_list_box(const char* label, const Vec2& size = Vec2(0, 0));
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
	void empty_state(const StateOptions& options);
	void loading_state(const char* text = "Loading...");
	void error_state(const char* message, const char* title = "Error");
}// namespace Trinex::UI
