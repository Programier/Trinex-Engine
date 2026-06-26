#pragma once
#include <Core/etl/function.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/etl/vector.hpp>
#include <Core/math/vector.hpp>

namespace Trinex
{
	class Window;
	class RHITexture;
	class RHISampler;
}// namespace Trinex

namespace Trinex::UI
{
	using Vec2      = Vector2f;
	using Vec3      = Vector3f;
	using Vec4      = Vector4f;
	using Action    = Function<void()>;
	using ActionRef = FunctionRef<void()>;

	class ID
	{
	public:
		using ValueType = i32;

	private:
		ValueType m_id;

	public:
		constexpr ID() : m_id(0) {}
		constexpr ID(ValueType id) : m_id(id) {}

		static ID from(StringView id);
		static ID from(const void* id);
		static ID from(ValueType id);

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
			Linear    = 0,
			InQuad    = 1,
			OutQuad   = 2,
			InOutQuad = 3,
			OutCubic  = 4,
			OutBack   = 5,
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
		};

		trinex_enum_struct(Key);
	};

	struct MouseButton {
		enum Enum : u8
		{
			Left   = 0,
			Right  = 1,
			Middle = 2,
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

	struct StyleColor {
		enum Enum : u8
		{
			Text,
			TextDisabled,
			WindowBg,
			ChildBg,
			PopupBg,
			Border,
			BorderShadow,
			FrameBg,
			FrameBgHovered,
			FrameBgActive,
			TitleBg,
			TitleBgActive,
			TitleBgCollapsed,
			MenuBarBg,
			ScrollbarBg,
			ScrollbarGrab,
			ScrollbarGrabHovered,
			ScrollbarGrabActive,
			CheckMark,
			SliderGrab,
			SliderGrabActive,
			Button,
			ButtonHovered,
			ButtonActive,
			Header,
			HeaderHovered,
			HeaderActive,
			Separator,
			SeparatorHovered,
			SeparatorActive,
			ResizeGrip,
			ResizeGripHovered,
			ResizeGripActive,
			InputTextCursor,
			TabHovered,
			Tab,
			TabSelected,
			TabSelectedOverline,
			TabDimmed,
			TabDimmedSelected,
			TabDimmedSelectedOverline,
			DockingPreview,
			DockingEmptyBg,
			PlotLines,
			PlotLinesHovered,
			PlotHistogram,
			PlotHistogramHovered,
			TableHeaderBg,
			TableBorderStrong,
			TableBorderLight,
			TableRowBg,
			TableRowBgAlt,
			TextLink,
			TextSelectedBg,
			TreeLines,
			DragDropTarget,
			DragDropTargetBg,
			UnsavedMarker,
			NavCursor,
			NavWindowingHighlight,
			NavWindowingDimBg,
			ModalWindowDimBg,
		};

		trinex_enum_struct(StyleColor);
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

	using DockID = ID;

	struct DockWindowFlags {
		enum Enum : u32
		{
			Undefined          = 0,
			NoSplit            = 1u << 0,
			NoResize           = 1u << 1,
			AutoHideTabBar     = 1u << 2,
			NoUndocking        = 1u << 3,
			NoTabBar           = 1u << 4,
			HiddenTabBar       = 1u << 5,
			NoWindowMenuButton = 1u << 6,
			NoCloseButton      = 1u << 7,
			NoDockingOverMe    = 1u << 8,
			NoDockingOverOther = 1u << 9,
			NoDockingOverEmpty = 1u << 10,
			AlwaysTabBar       = 1u << 11,
			AllowUnclassed     = 1u << 12,
			NoDocking          = NoDockingOverMe | NoDockingOverOther | NoDockingOverEmpty | NoSplit
		};

		trinex_bitfield_enum_struct(DockWindowFlags, u32);
	};

	struct DockTabFlags {
		enum Enum : u16
		{
			Undefined                    = 0,
			Unsaved                      = 1u << 0,
			SetSelected                  = 1u << 1,
			NoCloseWithMiddleMouseButton = 1u << 2,
			NoTooltip                    = 1u << 3,
			NoReorder                    = 1u << 4,
			Leading                      = 1u << 5,
			Trailing                     = 1u << 6,
			NoAssumedClosure             = 1u << 7
		};

		trinex_bitfield_enum_struct(DockTabFlags, u16);
	};

	struct DockNodeFlags {
		enum Enum : u32
		{
			Undefined                = 0,
			KeepAliveOnly            = 1u << 0,
			NoDockingOverCentralNode = 1u << 2,
			PassthruCentralNode      = 1u << 3,
			NoSplit                  = 1u << 4,
			NoResize                 = 1u << 5,
			AutoHideTabBar           = 1u << 6,
			NoUndocking              = 1u << 7,
			NoTabBar                 = 1u << 12,
			HiddenTabBar             = 1u << 13,
			NoWindowMenuButton       = 1u << 14,
			NoCloseButton            = 1u << 15,
			NoDockingOverMe          = 1u << 20,
			NoDockingOverOther       = 1u << 21,
			NoDockingOverEmpty       = 1u << 22,
			NoDocking                = NoDockingOverMe | NoDockingOverOther | NoDockingOverEmpty | NoSplit
		};

		trinex_bitfield_enum_struct(DockNodeFlags, u32);
	};

	struct DockSplitDir {
		enum Enum : u8
		{
			Left,
			Right,
			Up,
			Down,
		};

		trinex_enum_struct(DockSplitDir);
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
		};

		trinex_enum_struct(ConfirmResult);
	};

	struct DrawList {
		enum Enum : u8
		{
			Default,
			Background,
			Foreground,
		};

		trinex_enum_struct(DrawList);
	};

	struct DragDropPayload {
		const void* data = nullptr;
		int data_size    = 0;
		bool preview     = false;
		bool delivery    = false;
	};

	struct DockBuilderSplitResult {
		ID remainder = ID(0);
		ID child     = ID(0);

		inline operator ID() const { return child; }
		inline ID value() const { return child; }
		inline ID value(ID* out_remainder) const
		{
			if (out_remainder)
				(*out_remainder) = remainder;
			return child;
		}
	};

	struct DockLayoutOptions {
		DockID id           = DockID();
		Vec2 size           = Vec2(0.0f, 0.0f);
		DockNodeFlags flags = DockNodeFlags::Undefined;
		bool reset          = false;
	};

	struct DockPlacement {
		DockID id             = DockID();
		const char* id_text   = nullptr;
		Condition condition   = Condition::Undefined;
		DockWindowFlags flags = DockWindowFlags::Undefined;
	};

	struct WindowPlacement {
		Vec2 position                = Vec2(0.0f, 0.0f);
		Vec2 size                    = Vec2(0.0f, 0.0f);
		Vec2 min_size                = Vec2(0.0f, 0.0f);
		Vec2 max_size                = Vec2(0.0f, 0.0f);
		Condition position_condition = Condition::Undefined;
		Condition size_condition     = Condition::Undefined;
	};

	struct WindowState {
		bool focus                    = false;
		bool collapsed                = false;
		Condition collapsed_condition = Condition::Undefined;
	};

	struct WindowTabOptions {
		DockTabFlags flags  = DockTabFlags::Undefined;
		Vec4 color          = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
		const char* tooltip = nullptr;
	};

	class DockLayoutBuilder
	{
	public:
		using Dir    = DockSplitDir;
		using Result = DockBuilderSplitResult;

	private:
		struct NamedDock {
			String id;
			DockID dock = DockID();
		};

		DockID m_root = DockID();
		DockID m_main = DockID();
		Vector<NamedDock> m_named;

	public:
		bool exists() const;
		DockLayoutBuilder& bind(const char* id, DockID dock);
		DockLayoutBuilder& flags(DockID dock, DockNodeFlags flags);
		DockLayoutBuilder& flags(const char* id, DockNodeFlags flags);
		DockID find(const char* id) const;
		DockID require(const char* id) const;
		bool has(const char* id) const;
		Result split(DockID dock, DockSplitDir dir, float ratio, const char* id = nullptr);
		Result split(DockID dock, DockSplitDir dir, float ratio, const char* remainder_id, const char* child_id);
		DockID crop(DockID& dock, DockSplitDir dir, float ratio, const char* id = nullptr);
		DockID crop(DockID& dock, DockSplitDir dir, float ratio, const char* remainder_id, const char* child_id);
		DockID dock(const char* window_name, DockID dock_id);
		DockID dock(const char* window_name, const char* dock_id);

		bool begin(Vec2 size = {}, DockNodeFlags flags = DockNodeFlags::Undefined);
		bool begin(DockID root, Vec2 size = {}, DockNodeFlags flags = DockNodeFlags::Undefined);
		DockLayoutBuilder& end();

		inline DockID root() const { return m_root; }
		inline DockID main() const { return m_main; }
		inline DockLayoutBuilder& main(DockID id) { trinex_this_return(m_main = id ? id : m_root); }

		inline DockID dock(const char* window_name) { return dock(window_name, m_main); }
		inline Result split(DockSplitDir dir, float ratio, const char* id = nullptr) { return split(m_main, dir, ratio, id); }
		inline Result split(DockSplitDir dir, float ratio, const char* remainder_id, const char* child_id)
		{
			return split(m_main, dir, ratio, remainder_id, child_id);
		}
	};

	struct WindowOptions {
		WindowPlacement placement;
		WindowState state;
		DockPlacement dock;
		WindowTabOptions tab;
		WindowFlags flags = WindowFlags::Undefined;
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

	struct Shadow {
		Vec2 offset  = Vec2(0.0f, 4.0f);
		float blur   = 16.0f;
		float spread = 0.0f;
		Vec4 color   = Vec4(0.0f, 0.0f, 0.0f, 0.22f);
	};

	struct BlurOptions {
		float radius        = 12.0f;
		float sigma         = 5.0f;
		float spread        = 0.0f;
		float rounding      = -1.0f;
		float noise_opacity = 0.f;
		float noise_scale   = 1.0f;
		Vec4 tint           = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
	};

	struct Style {
		float animation_speed = 12.0f;
		float rounding        = 8.0f;
		float border_size     = 1.0f;
		float frame_height    = 32.0f;
		float padding         = 10.0f;
		float spacing         = 8.0f;
		float alpha           = 1.0f;
		Vec2 hover_padding    = Vec2(2.0f, 2.0f);
		BlurOptions blur;
		Shadow shadow;
		ColorTheme colors;
	};

	struct PanelOptions {
		Vec2 size             = Vec2(0.0f, 0.0f);
		bool border           = true;
		bool background       = true;
		float rounding        = -1.0f;
		Vec4 background_color = Vec4(0, 0, 0, 0);
	};

	struct GlassOptions {
		float opacity = 0.72f;

		Vec4 tint         = Vec4(0.10f, 0.12f, 0.16f, 0.65f);
		Vec4 border_color = Vec4(0, 0, 0, 0);
		Vec4 highlight    = Vec4(1, 1, 1, 0.08f);

		bool border        = true;
		bool background    = true;
		bool highlight_top = true;

		float rounding = -1.0f;
		float padding  = -1.0f;
	};

	struct CardOptions {
		Vec2 size              = Vec2(0.0f, 0.0f);
		const char* subtitle   = nullptr;
		const char* icon       = nullptr;
		const char* right_text = nullptr;

		bool border     = true;
		bool background = true;
		bool hoverable  = true;
		bool selected   = false;
		bool disabled   = false;

		float rounding  = -1.0f;
		float padding   = -1.0f;
		float spacing   = -1.0f;
		float elevation = 1.0f;

		Vec4 accent           = Vec4(0, 0, 0, 0);
		Vec4 background_color = Vec4(0, 0, 0, 0);
		Vec4 border_color     = Vec4(0, 0, 0, 0);
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
		ID id               = ID(0);
	};

	struct NotificationOptions {
		NotificationKind kind    = NotificationKind::Info;
		float duration           = 3.0f;
		const char* title        = nullptr;
		const char* action_label = nullptr;
		Action action;
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

	struct HeroOptions {
		const char* icon         = nullptr;
		const char* title        = nullptr;
		const char* description  = nullptr;
		const char* action_label = nullptr;

		Vec2 size   = Vec2(0.0f, 0.0f);
		Vec4 accent = Vec4(0, 0, 0, 0);

		bool border     = true;
		bool background = true;
		bool centered   = true;
		bool disabled   = false;

		float rounding  = -1.0f;
		float padding   = -1.0f;
		float spacing   = -1.0f;
		float elevation = 0.0f;
	};

	struct Command {
		const char* id          = nullptr;
		const char* name        = nullptr;
		const char* description = nullptr;
		const char* shortcut    = nullptr;
		const char* icon        = nullptr;
		Action action;
	};

	template<typename T>
	concept TriviallyStored = std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;

	using PaintFunction = void (*)(void* userdata);

	struct Context;

	enum class FontFamily : u8
	{
		Default = 0,
		Text,
		Icons,
	};

	class Widget
	{
	public:
		virtual ~Widget();

		virtual void on_init();
		virtual void on_render();
		virtual void on_close();
	};

	template<typename T>
	class UniqueWidget : public T
	{
	public:
		using T::T;

		template<typename... Args>
		static UniqueWidget* create(const Args&... args)
		{
			return trx_new UniqueWidget<T>(args...);
		}

		virtual void on_close() override
		{
			T::on_close();
			trx_delete this;
		}
	};
}// namespace Trinex::UI
