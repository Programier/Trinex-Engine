#pragma once
#include <Core/math/vector.hpp>

namespace Trinex
{
	class Window;
	class RHITexture;
	class RHISampler;
	class RHIContext;
}// namespace Trinex

namespace Trinex::UI
{
	using Vec2  = Vector2f;
	using Vec3  = Vector3f;
	using Vec4  = Vector4f;
	using Color = Vector4f;

	struct Axis {
		enum Enum : u8
		{
			X,
			Y,
		};

		trinex_enum_struct(Axis);
	};

	struct Ease {
		enum Enum : u8
		{
			Linear = 0,

			InQuad    = 1,
			OutQuad   = 2,
			InOutQuad = 3,

			InCubic    = 4,
			OutCubic   = 5,
			InOutCubic = 6,

			InExpo    = 7,
			OutExpo   = 8,
			InOutExpo = 9,

			OutBack = 10,
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

	struct StyleVar {
		enum Enum : u8
		{
			Alpha,
			DisabledAlpha,
			WindowPadding,
			WindowRounding,
			WindowBorderSize,
			WindowMinSize,
			WindowTitleAlign,
			ChildRounding,
			ChildBorderSize,
			PopupRounding,
			PopupBorderSize,
			FramePadding,
			FrameRounding,
			FrameBorderSize,
			ItemSpacing,
			ItemInnerSpacing,
			IndentSpacing,
			CellPadding,
			ScrollbarSize,
			ScrollbarRounding,
			ScrollbarPadding,
			GrabMinSize,
			GrabRounding,
			ImageRounding,
			ImageBorderSize,
			TabRounding,
			TabBorderSize,
			TabMinWidthBase,
			TabMinWidthShrink,
			TabBarBorderSize,
			TabBarOverlineSize,
			TableAngledHeadersAngle,
			TableAngledHeadersTextAlign,
			TreeLinesSize,
			TreeLinesRounding,
			ButtonTextAlign,
			SelectableTextAlign,
			SeparatorSize,
			SeparatorTextBorderSize,
			SeparatorTextAlign,
			SeparatorTextPadding,
			DockingSeparatorSize,
			LayoutAlign,
		};

		trinex_enum_struct(StyleVar);
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

	struct LayerCompositeMode {
		enum Enum : u8
		{
			Undefined  = 0,
			AlphaBlend = 1,
			Copy       = 2,
			Additive   = 3,
			Custom     = 4,
		};

		trinex_enum_struct(LayerCompositeMode);
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
		enum Enum : u32
		{
			Undefined                  = 0,
			Resizable                  = 1 << 0,
			Reorderable                = 1 << 1,
			Hideable                   = 1 << 2,
			Sortable                   = 1 << 3,
			NoSavedSettings            = 1 << 4,
			ContextMenuInBody          = 1 << 5,
			RowBg                      = 1 << 6,
			BordersInnerH              = 1 << 7,
			BordersOuterH              = 1 << 8,
			BordersInnerV              = 1 << 9,
			BordersOuterV              = 1 << 10,
			BordersH                   = BordersInnerH | BordersOuterH,
			BordersV                   = BordersInnerV | BordersOuterV,
			BordersInner               = BordersInnerV | BordersInnerH,
			BordersOuter               = BordersOuterV | BordersOuterH,
			Borders                    = BordersInner | BordersOuter,
			NoBordersInBody            = 1 << 11,
			NoBordersInBodyUntilResize = 1 << 12,
			SizingFixedFit             = 1 << 13,
			SizingFixedSame            = 2 << 13,
			SizingStretchProp          = 3 << 13,
			SizingStretchSame          = 4 << 13,
			NoHostExtendX              = 1 << 16,
			NoHostExtendY              = 1 << 17,
			NoKeepColumnsVisible       = 1 << 18,
			PreciseWidths              = 1 << 19,
			NoClip                     = 1 << 20,
			PadOuterX                  = 1 << 21,
			NoPadOuterX                = 1 << 22,
			NoPadInnerX                = 1 << 23,
			ScrollX                    = 1 << 24,
			ScrollY                    = 1 << 25,
			SortMulti                  = 1 << 26,
			SortTristate               = 1 << 27,
			HighlightHoveredColumn     = 1 << 28,
		};

		trinex_bitfield_enum_struct(TableFlags, u32);
	};

	struct DrawFlags {
		enum Enum : u16
		{
			Undefined               = 0,
			Closed                  = 1 << 0,
			RoundCornersTopLeft     = 1 << 4,
			RoundCornersTopRight    = 1 << 5,
			RoundCornersBottomLeft  = 1 << 6,
			RoundCornersBottomRight = 1 << 7,
			RoundCornersNone        = 1 << 8,
			RoundCornersTop         = RoundCornersTopLeft | RoundCornersTopRight,
			RoundCornersBottom      = RoundCornersBottomLeft | RoundCornersBottomRight,
			RoundCornersLeft        = RoundCornersBottomLeft | RoundCornersTopLeft,
			RoundCornersRight       = RoundCornersBottomRight | RoundCornersTopRight,
			RoundCornersAll = RoundCornersTopLeft | RoundCornersTopRight | RoundCornersBottomLeft | RoundCornersBottomRight,
		};

		trinex_bitfield_enum_struct(DrawFlags, u16);
	};

	struct TableColumnFlags {
		enum Enum : u32
		{
			Undefined            = 0,
			Disabled             = 1 << 0,
			DefaultHide          = 1 << 1,
			DefaultSort          = 1 << 2,
			WidthStretch         = 1 << 3,
			WidthFixed           = 1 << 4,
			NoResize             = 1 << 5,
			NoReorder            = 1 << 6,
			NoHide               = 1 << 7,
			NoClip               = 1 << 8,
			NoSort               = 1 << 9,
			NoSortAscending      = 1 << 10,
			NoSortDescending     = 1 << 11,
			NoHeaderLabel        = 1 << 12,
			NoHeaderWidth        = 1 << 13,
			PreferSortAscending  = 1 << 14,
			PreferSortDescending = 1 << 15,
			IndentEnable         = 1 << 16,
			IndentDisable        = 1 << 17,
			AngledHeader         = 1 << 18,
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

	struct ButtonFlags {
		enum Enum : u32
		{
			Undefined                     = 0,
			MouseButtonLeft               = 1 << 0,
			MouseButtonRight              = 1 << 1,
			MouseButtonMiddle             = 1 << 2,
			EnableNav                     = 1 << 3,
			PressedOnClick                = 1 << 4,
			PressedOnClickRelease         = 1 << 5,
			PressedOnClickReleaseAnywhere = 1 << 6,
			PressedOnRelease              = 1 << 7,
			PressedOnDoubleClick          = 1 << 8,
			PressedOnDragDropHold         = 1 << 9,
			FlattenChildren               = 1 << 11,
			AllowOverlap                  = 1 << 12,
			AlignTextBaseLine             = 1 << 15,
			NoKeyModsAllowed              = 1 << 16,
			NoHoldingActiveId             = 1 << 17,
			NoNavFocus                    = 1 << 18,
			NoHoveredOnFocus              = 1 << 19,
			NoSetKeyOwner                 = 1 << 20,
			NoTestKeyOwner                = 1 << 21,
			NoFocus                       = 1 << 22,
		};

		trinex_bitfield_enum_struct(ButtonFlags, u32);
	};

	struct PanelFlags {
		enum Enum : u16
		{
			Undefined              = 0,
			Borders                = 1 << 0,
			AlwaysUseWindowPadding = 1 << 1,
			ResizeX                = 1 << 2,
			ResizeY                = 1 << 3,
			AutoResizeX            = 1 << 4,
			AutoResizeY            = 1 << 5,
			AlwaysAutoResize       = 1 << 6,
			FrameStyle             = 1 << 7,
			NavFlattened           = 1 << 8,
		};

		trinex_bitfield_enum_struct(PanelFlags, u32);
	};

	struct RenderScaleFlags {
		enum Enum : u8
		{
			Undefined               = 0,
			StartFromLastItemBounds = 1 << 0,
		};

		trinex_bitfield_enum_struct(RenderScaleFlags, u8);
	};

	struct LayerFlags {
		enum Enum : u8
		{
			Undefined           = 0,
			ClearOnPush         = 1 << 0,
			IncludeWindowBg     = 1 << 1,
			IncludeWindowBorder = 1 << 2,
			IncludeWindowDecor  = 1 << 3,
			ClipToWindowRect    = 1 << 4,
			CaptureWholeWindow  = 1 << 5,
			CompositeOnPop      = 1 << 6,
		};

		trinex_bitfield_enum_struct(LayerFlags, u8);
	};

	struct PaintFlags {
		enum Enum : u8
		{
			Undefined     = 0,
			SetupViewport = 1 << 0,
			SetupScissor  = 1 << 1,
			PushFront     = 1 << 2,
		};
		trinex_bitfield_enum_struct(PaintFlags, u8);
	};
}// namespace Trinex::UI
