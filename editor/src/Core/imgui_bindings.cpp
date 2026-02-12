#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/algorithm.hpp>
#include <Core/etl/templates.hpp>
#include <Graphics/imgui.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_binder.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_pointer.hpp>
#include <angelscript.h>
#include <imgui_internal.h>
#include <scriptarray.h>

struct ImDrawList;
struct ImFont;
struct ImFontAtlas;
struct ImFontConfig;
struct ImFontGlyph;
struct ImFontGlyphRangesBuilder;
struct ImColor;
struct ImGuiIO;
struct ImGuiKeyData;
struct ImGuiListClipper;
struct ImGuiOnceUponAFrame;
struct ImGuiPayload;
struct ImGuiPlatformIO;
struct ImGuiPlatformMonitor;
struct ImGuiPlatformImeData;
struct ImGuiSizeCallbackData;
struct ImGuiStorage;
struct ImGuiTableSortSpecs;
struct ImGuiTableColumnSortSpecs;
struct ImGuiTextBuffer;
struct ImGuiTextFilter;
struct ImGuiViewport;
struct ImGuiWindowClass;

// clang-format off
struct ImVec3
{
	float												x, y, z;
	constexpr ImVec3()									: x(0.0f), y(0.0f), z(0.0f) { }
	constexpr ImVec3(float _x, float _y, float _z)		: x(_x), y(_y), z(_z) { }
};

struct ImIVec2
{
										int x, y;
	constexpr ImIVec2()					: x(0), y(0) { }
	constexpr ImIVec2(int _x, int _y)	: x(_x), y(_y) { }
};

struct ImIVec3
{
												int x, y, z;
	constexpr ImIVec3()							: x(0), y(0), z(0) { }
	constexpr ImIVec3(int _x, int _y, int _z)	: x(_x), y(_y), z(_z) { }
};

struct ImIVec4
{
														int x, y, z, w;
	constexpr ImIVec4()									: x(0), y(0), z(0), w(0) { }
	constexpr ImIVec4(int _x, int _y, int _z, int _w)	: x(_x), y(_y), z(_z), w(_z) { }
};

struct NullableString {};
// clang-format on

namespace Engine
{
#define new_enum_v(a, b) new_enum.set(#b, a##_##b)
#define new_enum_v2(a, b) new_enum.set(#b, a##b)
	template<typename T>
	struct ImGuiRemapper {
		using ArgType = T;
		using RetType = T;

		static T convert(T value) { return value; }
	};

	template<>
	struct ImGuiRemapper<void> {
		using ArgType = void;
		using RetType = void;
	};

	template<>
	struct ImGuiRemapper<const char*> {
		using ArgType = const String&;
		using RetType = String;

		static const char* convert(ArgType value) { return value.c_str(); }
	};

	template<>
	struct ImGuiRemapper<NullableString> {
		using ArgType = const String&;
		using RetType = String;

		static const char* convert(ArgType value)
		{
			if (value.empty())
				return nullptr;
			return value.c_str();
		}
	};

	template<typename T>
	struct ImGuiRemapper<T*> {
		using ArgType = ScriptPointer;
		using RetType = ScriptPointer;

		static T* convert(const ArgType& value) { return value.as<T>(); }
	};

	template<typename T, std::size_t N>
	struct ImGuiRemapper<T[N]> {
		struct Value {
			T values[N];
		};

		using ArgType = Value&;
		using RetType = Value;

		static T* convert(ArgType arg) { return arg.values; }
	};

	template<auto func, typename... Overrides>
	using ImBinder = Binder::ScriptBinder<ImGuiRemapper, func, Overrides...>;

	template<int idx, typename T>
	using ImArg = Binder::Arg<idx, T>;

	static void register_enums()
	{
		{
			ScriptEnumRegistrar new_enum("ImGuiWindowFlags");
			new_enum_v(ImGuiWindowFlags, None);
			new_enum_v(ImGuiWindowFlags, NoTitleBar);
			new_enum_v(ImGuiWindowFlags, NoResize);
			new_enum_v(ImGuiWindowFlags, NoMove);
			new_enum_v(ImGuiWindowFlags, NoScrollbar);
			new_enum_v(ImGuiWindowFlags, NoScrollWithMouse);
			new_enum_v(ImGuiWindowFlags, NoCollapse);
			new_enum_v(ImGuiWindowFlags, AlwaysAutoResize);
			new_enum_v(ImGuiWindowFlags, NoBackground);
			new_enum_v(ImGuiWindowFlags, NoSavedSettings);
			new_enum_v(ImGuiWindowFlags, NoMouseInputs);
			new_enum_v(ImGuiWindowFlags, MenuBar);
			new_enum_v(ImGuiWindowFlags, HorizontalScrollbar);
			new_enum_v(ImGuiWindowFlags, NoFocusOnAppearing);
			new_enum_v(ImGuiWindowFlags, NoBringToFrontOnFocus);
			new_enum_v(ImGuiWindowFlags, AlwaysVerticalScrollbar);
			new_enum_v(ImGuiWindowFlags, AlwaysHorizontalScrollbar);
			new_enum_v(ImGuiWindowFlags, NoNavInputs);
			new_enum_v(ImGuiWindowFlags, NoNavFocus);
			new_enum_v(ImGuiWindowFlags, UnsavedDocument);
			new_enum_v(ImGuiWindowFlags, NoDocking);
			new_enum_v(ImGuiWindowFlags, NoNav);
			new_enum_v(ImGuiWindowFlags, NoDecoration);
			new_enum_v(ImGuiWindowFlags, NoInputs);
			new_enum_v(ImGuiWindowFlags, NavFlattened);
			new_enum_v(ImGuiWindowFlags, ChildWindow);
			new_enum_v(ImGuiWindowFlags, Tooltip);
			new_enum_v(ImGuiWindowFlags, Popup);
			new_enum_v(ImGuiWindowFlags, Modal);
			new_enum_v(ImGuiWindowFlags, ChildMenu);
			new_enum_v(ImGuiWindowFlags, DockNodeHost);
			new_enum_v(ImGuiWindowFlags, AlwaysUseWindowPadding);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiChildFlags");
			new_enum_v(ImGuiChildFlags, None);
			new_enum_v(ImGuiChildFlags, Border);
			new_enum_v(ImGuiChildFlags, AlwaysUseWindowPadding);
			new_enum_v(ImGuiChildFlags, ResizeX);
			new_enum_v(ImGuiChildFlags, ResizeY);
			new_enum_v(ImGuiChildFlags, AutoResizeX);
			new_enum_v(ImGuiChildFlags, AutoResizeY);
			new_enum_v(ImGuiChildFlags, AlwaysAutoResize);
			new_enum_v(ImGuiChildFlags, FrameStyle);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiInputTextFlags");
			new_enum_v(ImGuiInputTextFlags, None);
			new_enum_v(ImGuiInputTextFlags, CharsDecimal);
			new_enum_v(ImGuiInputTextFlags, CharsHexadecimal);
			new_enum_v(ImGuiInputTextFlags, CharsUppercase);
			new_enum_v(ImGuiInputTextFlags, CharsNoBlank);
			new_enum_v(ImGuiInputTextFlags, AutoSelectAll);
			new_enum_v(ImGuiInputTextFlags, EnterReturnsTrue);
			new_enum_v(ImGuiInputTextFlags, CallbackCompletion);
			new_enum_v(ImGuiInputTextFlags, CallbackHistory);
			new_enum_v(ImGuiInputTextFlags, CallbackAlways);
			new_enum_v(ImGuiInputTextFlags, CallbackCharFilter);
			new_enum_v(ImGuiInputTextFlags, AllowTabInput);
			new_enum_v(ImGuiInputTextFlags, CtrlEnterForNewLine);
			new_enum_v(ImGuiInputTextFlags, NoHorizontalScroll);
			new_enum_v(ImGuiInputTextFlags, AlwaysOverwrite);
			new_enum_v(ImGuiInputTextFlags, ReadOnly);
			new_enum_v(ImGuiInputTextFlags, Password);
			new_enum_v(ImGuiInputTextFlags, NoUndoRedo);
			new_enum_v(ImGuiInputTextFlags, CharsScientific);
			new_enum_v(ImGuiInputTextFlags, CallbackResize);
			new_enum_v(ImGuiInputTextFlags, CallbackEdit);
			new_enum_v(ImGuiInputTextFlags, EscapeClearsAll);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiTreeNodeFlags");
			new_enum_v(ImGuiTreeNodeFlags, None);
			new_enum_v(ImGuiTreeNodeFlags, Selected);
			new_enum_v(ImGuiTreeNodeFlags, Framed);
			new_enum_v(ImGuiTreeNodeFlags, AllowOverlap);
			new_enum_v(ImGuiTreeNodeFlags, NoTreePushOnOpen);
			new_enum_v(ImGuiTreeNodeFlags, NoAutoOpenOnLog);
			new_enum_v(ImGuiTreeNodeFlags, DefaultOpen);
			new_enum_v(ImGuiTreeNodeFlags, OpenOnDoubleClick);
			new_enum_v(ImGuiTreeNodeFlags, OpenOnArrow);
			new_enum_v(ImGuiTreeNodeFlags, Leaf);
			new_enum_v(ImGuiTreeNodeFlags, Bullet);
			new_enum_v(ImGuiTreeNodeFlags, FramePadding);
			new_enum_v(ImGuiTreeNodeFlags, SpanAvailWidth);
			new_enum_v(ImGuiTreeNodeFlags, SpanFullWidth);
			new_enum_v(ImGuiTreeNodeFlags, SpanAllColumns);
			new_enum_v(ImGuiTreeNodeFlags, NavLeftJumpsBackHere);
			new_enum_v(ImGuiTreeNodeFlags, CollapsingHeader);
			new_enum_v(ImGuiTreeNodeFlags, AllowItemOverlap);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiPopupFlags");
			new_enum_v(ImGuiPopupFlags, None);
			new_enum_v(ImGuiPopupFlags, MouseButtonLeft);
			new_enum_v(ImGuiPopupFlags, MouseButtonRight);
			new_enum_v(ImGuiPopupFlags, MouseButtonMiddle);
			new_enum_v(ImGuiPopupFlags, MouseButtonMask_);
			new_enum_v(ImGuiPopupFlags, MouseButtonDefault_);
			new_enum_v(ImGuiPopupFlags, NoOpenOverExistingPopup);
			new_enum_v(ImGuiPopupFlags, NoOpenOverItems);
			new_enum_v(ImGuiPopupFlags, AnyPopupId);
			new_enum_v(ImGuiPopupFlags, AnyPopupLevel);
			new_enum_v(ImGuiPopupFlags, AnyPopup);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiSelectableFlags");
			new_enum_v(ImGuiSelectableFlags, None);
			new_enum_v(ImGuiSelectableFlags, DontClosePopups);
			new_enum_v(ImGuiSelectableFlags, SpanAllColumns);
			new_enum_v(ImGuiSelectableFlags, AllowDoubleClick);
			new_enum_v(ImGuiSelectableFlags, Disabled);
			new_enum_v(ImGuiSelectableFlags, AllowOverlap);
			new_enum_v(ImGuiSelectableFlags, AllowItemOverlap);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiComboFlags");
			new_enum_v(ImGuiComboFlags, None);
			new_enum_v(ImGuiComboFlags, PopupAlignLeft);
			new_enum_v(ImGuiComboFlags, HeightSmall);
			new_enum_v(ImGuiComboFlags, HeightRegular);
			new_enum_v(ImGuiComboFlags, HeightLarge);
			new_enum_v(ImGuiComboFlags, HeightLargest);
			new_enum_v(ImGuiComboFlags, NoArrowButton);
			new_enum_v(ImGuiComboFlags, NoPreview);
			new_enum_v(ImGuiComboFlags, WidthFitPreview);
			new_enum_v(ImGuiComboFlags, HeightMask_);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiTabBarFlags");
			new_enum_v(ImGuiTabBarFlags, None);
			new_enum_v(ImGuiTabBarFlags, Reorderable);
			new_enum_v(ImGuiTabBarFlags, AutoSelectNewTabs);
			new_enum_v(ImGuiTabBarFlags, TabListPopupButton);
			new_enum_v(ImGuiTabBarFlags, NoCloseWithMiddleMouseButton);
			new_enum_v(ImGuiTabBarFlags, NoTabListScrollingButtons);
			new_enum_v(ImGuiTabBarFlags, NoTooltip);
			new_enum_v(ImGuiTabBarFlags, FittingPolicyResizeDown);
			new_enum_v(ImGuiTabBarFlags, FittingPolicyScroll);
			new_enum_v(ImGuiTabBarFlags, FittingPolicyMask_);
			new_enum_v(ImGuiTabBarFlags, FittingPolicyDefault_);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiTabItemFlags");
			new_enum_v(ImGuiTabItemFlags, None);
			new_enum_v(ImGuiTabItemFlags, UnsavedDocument);
			new_enum_v(ImGuiTabItemFlags, SetSelected);
			new_enum_v(ImGuiTabItemFlags, NoCloseWithMiddleMouseButton);
			new_enum_v(ImGuiTabItemFlags, NoPushId);
			new_enum_v(ImGuiTabItemFlags, NoTooltip);
			new_enum_v(ImGuiTabItemFlags, NoReorder);
			new_enum_v(ImGuiTabItemFlags, Leading);
			new_enum_v(ImGuiTabItemFlags, Trailing);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiTableFlags");
			new_enum_v(ImGuiTableFlags, None);
			new_enum_v(ImGuiTableFlags, Resizable);
			new_enum_v(ImGuiTableFlags, Reorderable);
			new_enum_v(ImGuiTableFlags, Hideable);
			new_enum_v(ImGuiTableFlags, Sortable);
			new_enum_v(ImGuiTableFlags, NoSavedSettings);
			new_enum_v(ImGuiTableFlags, ContextMenuInBody);
			new_enum_v(ImGuiTableFlags, RowBg);
			new_enum_v(ImGuiTableFlags, BordersInnerH);
			new_enum_v(ImGuiTableFlags, BordersOuterH);
			new_enum_v(ImGuiTableFlags, BordersInnerV);
			new_enum_v(ImGuiTableFlags, BordersOuterV);
			new_enum_v(ImGuiTableFlags, BordersH);
			new_enum_v(ImGuiTableFlags, BordersV);
			new_enum_v(ImGuiTableFlags, BordersInner);
			new_enum_v(ImGuiTableFlags, BordersOuter);
			new_enum_v(ImGuiTableFlags, Borders);
			new_enum_v(ImGuiTableFlags, NoBordersInBody);
			new_enum_v(ImGuiTableFlags, NoBordersInBodyUntilResize);
			new_enum_v(ImGuiTableFlags, SizingFixedFit);
			new_enum_v(ImGuiTableFlags, SizingFixedSame);
			new_enum_v(ImGuiTableFlags, SizingStretchProp);
			new_enum_v(ImGuiTableFlags, SizingStretchSame);
			new_enum_v(ImGuiTableFlags, NoHostExtendX);
			new_enum_v(ImGuiTableFlags, NoHostExtendY);
			new_enum_v(ImGuiTableFlags, NoKeepColumnsVisible);
			new_enum_v(ImGuiTableFlags, PreciseWidths);
			new_enum_v(ImGuiTableFlags, NoClip);
			new_enum_v(ImGuiTableFlags, PadOuterX);
			new_enum_v(ImGuiTableFlags, NoPadOuterX);
			new_enum_v(ImGuiTableFlags, NoPadInnerX);
			new_enum_v(ImGuiTableFlags, ScrollX);
			new_enum_v(ImGuiTableFlags, ScrollY);
			new_enum_v(ImGuiTableFlags, SortMulti);
			new_enum_v(ImGuiTableFlags, SortTristate);
			new_enum_v(ImGuiTableFlags, HighlightHoveredColumn);
			new_enum_v(ImGuiTableFlags, SizingMask_);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiTableColumnFlags");
			new_enum_v(ImGuiTableColumnFlags, None);
			new_enum_v(ImGuiTableColumnFlags, Disabled);
			new_enum_v(ImGuiTableColumnFlags, DefaultHide);
			new_enum_v(ImGuiTableColumnFlags, DefaultSort);
			new_enum_v(ImGuiTableColumnFlags, WidthStretch);
			new_enum_v(ImGuiTableColumnFlags, WidthFixed);
			new_enum_v(ImGuiTableColumnFlags, NoResize);
			new_enum_v(ImGuiTableColumnFlags, NoReorder);
			new_enum_v(ImGuiTableColumnFlags, NoHide);
			new_enum_v(ImGuiTableColumnFlags, NoClip);
			new_enum_v(ImGuiTableColumnFlags, NoSort);
			new_enum_v(ImGuiTableColumnFlags, NoSortAscending);
			new_enum_v(ImGuiTableColumnFlags, NoSortDescending);
			new_enum_v(ImGuiTableColumnFlags, NoHeaderLabel);
			new_enum_v(ImGuiTableColumnFlags, NoHeaderWidth);
			new_enum_v(ImGuiTableColumnFlags, PreferSortAscending);
			new_enum_v(ImGuiTableColumnFlags, PreferSortDescending);
			new_enum_v(ImGuiTableColumnFlags, IndentEnable);
			new_enum_v(ImGuiTableColumnFlags, IndentDisable);
			new_enum_v(ImGuiTableColumnFlags, AngledHeader);
			new_enum_v(ImGuiTableColumnFlags, IsEnabled);
			new_enum_v(ImGuiTableColumnFlags, IsVisible);
			new_enum_v(ImGuiTableColumnFlags, IsSorted);
			new_enum_v(ImGuiTableColumnFlags, IsHovered);
			new_enum_v(ImGuiTableColumnFlags, WidthMask_);
			new_enum_v(ImGuiTableColumnFlags, IndentMask_);
			new_enum_v(ImGuiTableColumnFlags, StatusMask_);
			new_enum_v(ImGuiTableColumnFlags, NoDirectResize_);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiTableRowFlags");
			new_enum_v(ImGuiTableRowFlags, None);
			new_enum_v(ImGuiTableRowFlags, Headers);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiTableBgTarget");
			new_enum_v(ImGuiTableBgTarget, None);
			new_enum_v(ImGuiTableBgTarget, RowBg0);
			new_enum_v(ImGuiTableBgTarget, RowBg1);
			new_enum_v(ImGuiTableBgTarget, CellBg);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiFocusedFlags");
			new_enum_v(ImGuiFocusedFlags, None);
			new_enum_v(ImGuiFocusedFlags, ChildWindows);
			new_enum_v(ImGuiFocusedFlags, RootWindow);
			new_enum_v(ImGuiFocusedFlags, AnyWindow);
			new_enum_v(ImGuiFocusedFlags, NoPopupHierarchy);
			new_enum_v(ImGuiFocusedFlags, DockHierarchy);
			new_enum_v(ImGuiFocusedFlags, RootAndChildWindows);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiHoveredFlags");
			new_enum_v(ImGuiHoveredFlags, None);
			new_enum_v(ImGuiHoveredFlags, ChildWindows);
			new_enum_v(ImGuiHoveredFlags, RootWindow);
			new_enum_v(ImGuiHoveredFlags, AnyWindow);
			new_enum_v(ImGuiHoveredFlags, NoPopupHierarchy);
			new_enum_v(ImGuiHoveredFlags, DockHierarchy);
			new_enum_v(ImGuiHoveredFlags, AllowWhenBlockedByPopup);
			new_enum_v(ImGuiHoveredFlags, AllowWhenBlockedByActiveItem);
			new_enum_v(ImGuiHoveredFlags, AllowWhenOverlappedByItem);
			new_enum_v(ImGuiHoveredFlags, AllowWhenOverlappedByWindow);
			new_enum_v(ImGuiHoveredFlags, AllowWhenDisabled);
			new_enum_v(ImGuiHoveredFlags, NoNavOverride);
			new_enum_v(ImGuiHoveredFlags, AllowWhenOverlapped);
			new_enum_v(ImGuiHoveredFlags, RectOnly);
			new_enum_v(ImGuiHoveredFlags, RootAndChildWindows);
			new_enum_v(ImGuiHoveredFlags, ForTooltip);
			new_enum_v(ImGuiHoveredFlags, Stationary);
			new_enum_v(ImGuiHoveredFlags, DelayNone);
			new_enum_v(ImGuiHoveredFlags, DelayShort);
			new_enum_v(ImGuiHoveredFlags, DelayNormal);
			new_enum_v(ImGuiHoveredFlags, NoSharedDelay);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiDockNodeFlags");
			new_enum_v(ImGuiDockNodeFlags, None);
			new_enum_v(ImGuiDockNodeFlags, KeepAliveOnly);
			new_enum_v(ImGuiDockNodeFlags, NoDockingOverCentralNode);
			new_enum_v(ImGuiDockNodeFlags, PassthruCentralNode);
			new_enum_v(ImGuiDockNodeFlags, NoDockingSplit);
			new_enum_v(ImGuiDockNodeFlags, NoResize);
			new_enum_v(ImGuiDockNodeFlags, AutoHideTabBar);
			new_enum_v(ImGuiDockNodeFlags, NoUndocking);
			new_enum_v(ImGuiDockNodeFlags, NoSplit);
			new_enum_v(ImGuiDockNodeFlags, NoDockingInCentralNode);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiDragDropFlags");
			new_enum_v(ImGuiDragDropFlags, None);
			new_enum_v(ImGuiDragDropFlags, SourceNoPreviewTooltip);
			new_enum_v(ImGuiDragDropFlags, SourceNoDisableHover);
			new_enum_v(ImGuiDragDropFlags, SourceNoHoldToOpenOthers);
			new_enum_v(ImGuiDragDropFlags, SourceAllowNullID);
			new_enum_v(ImGuiDragDropFlags, SourceExtern);
			new_enum_v(ImGuiDragDropFlags, SourceAutoExpirePayload);
			new_enum_v(ImGuiDragDropFlags, AcceptBeforeDelivery);
			new_enum_v(ImGuiDragDropFlags, AcceptNoDrawDefaultRect);
			new_enum_v(ImGuiDragDropFlags, AcceptNoPreviewTooltip);
			new_enum_v(ImGuiDragDropFlags, AcceptPeekOnly);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiDataType");
			new_enum_v(ImGuiDataType, S8);
			new_enum_v(ImGuiDataType, U8);
			new_enum_v(ImGuiDataType, S16);
			new_enum_v(ImGuiDataType, U16);
			new_enum_v(ImGuiDataType, S32);
			new_enum_v(ImGuiDataType, U32);
			new_enum_v(ImGuiDataType, S64);
			new_enum_v(ImGuiDataType, U64);
			new_enum_v(ImGuiDataType, Float);
			new_enum_v(ImGuiDataType, Double);
			new_enum_v(ImGuiDataType, COUNT);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiDir");
			new_enum_v(ImGuiDir, None);
			new_enum_v(ImGuiDir, Left);
			new_enum_v(ImGuiDir, Right);
			new_enum_v(ImGuiDir, Up);
			new_enum_v(ImGuiDir, Down);
			new_enum_v(ImGuiDir, COUNT);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiSortDirection");
			new_enum_v(ImGuiSortDirection, None);
			new_enum_v(ImGuiSortDirection, Ascending);
			new_enum_v(ImGuiSortDirection, Descending);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiKey");
			new_enum_v(ImGuiKey, None);
			new_enum_v(ImGuiKey, Tab);
			new_enum_v(ImGuiKey, LeftArrow);
			new_enum_v(ImGuiKey, RightArrow);
			new_enum_v(ImGuiKey, UpArrow);
			new_enum_v(ImGuiKey, DownArrow);
			new_enum_v(ImGuiKey, PageUp);
			new_enum_v(ImGuiKey, PageDown);
			new_enum_v(ImGuiKey, Home);
			new_enum_v(ImGuiKey, End);
			new_enum_v(ImGuiKey, Insert);
			new_enum_v(ImGuiKey, Delete);
			new_enum_v(ImGuiKey, Backspace);
			new_enum_v(ImGuiKey, Space);
			new_enum_v(ImGuiKey, Enter);
			new_enum_v(ImGuiKey, Escape);
			new_enum_v(ImGuiKey, LeftCtrl);
			new_enum_v(ImGuiKey, LeftShift);
			new_enum_v(ImGuiKey, LeftAlt);
			new_enum_v(ImGuiKey, LeftSuper);
			new_enum_v(ImGuiKey, RightCtrl);
			new_enum_v(ImGuiKey, RightShift);
			new_enum_v(ImGuiKey, RightAlt);
			new_enum_v(ImGuiKey, RightSuper);
			new_enum_v(ImGuiKey, Menu);
			new_enum_v2(ImGuiKey, _0);
			new_enum_v2(ImGuiKey, _1);
			new_enum_v2(ImGuiKey, _2);
			new_enum_v2(ImGuiKey, _3);
			new_enum_v2(ImGuiKey, _4);
			new_enum_v2(ImGuiKey, _5);
			new_enum_v2(ImGuiKey, _6);
			new_enum_v2(ImGuiKey, _7);
			new_enum_v2(ImGuiKey, _8);
			new_enum_v2(ImGuiKey, _9);
			new_enum_v(ImGuiKey, A);
			new_enum_v(ImGuiKey, B);
			new_enum_v(ImGuiKey, C);
			new_enum_v(ImGuiKey, D);
			new_enum_v(ImGuiKey, E);
			new_enum_v(ImGuiKey, F);
			new_enum_v(ImGuiKey, G);
			new_enum_v(ImGuiKey, H);
			new_enum_v(ImGuiKey, I);
			new_enum_v(ImGuiKey, J);
			new_enum_v(ImGuiKey, K);
			new_enum_v(ImGuiKey, L);
			new_enum_v(ImGuiKey, M);
			new_enum_v(ImGuiKey, N);
			new_enum_v(ImGuiKey, O);
			new_enum_v(ImGuiKey, P);
			new_enum_v(ImGuiKey, Q);
			new_enum_v(ImGuiKey, R);
			new_enum_v(ImGuiKey, S);
			new_enum_v(ImGuiKey, T);
			new_enum_v(ImGuiKey, U);
			new_enum_v(ImGuiKey, V);
			new_enum_v(ImGuiKey, W);
			new_enum_v(ImGuiKey, X);
			new_enum_v(ImGuiKey, Y);
			new_enum_v(ImGuiKey, Z);
			new_enum_v(ImGuiKey, F1);
			new_enum_v(ImGuiKey, F2);
			new_enum_v(ImGuiKey, F3);
			new_enum_v(ImGuiKey, F4);
			new_enum_v(ImGuiKey, F5);
			new_enum_v(ImGuiKey, F6);
			new_enum_v(ImGuiKey, F7);
			new_enum_v(ImGuiKey, F8);
			new_enum_v(ImGuiKey, F9);
			new_enum_v(ImGuiKey, F10);
			new_enum_v(ImGuiKey, F11);
			new_enum_v(ImGuiKey, F12);
			new_enum_v(ImGuiKey, F13);
			new_enum_v(ImGuiKey, F14);
			new_enum_v(ImGuiKey, F15);
			new_enum_v(ImGuiKey, F16);
			new_enum_v(ImGuiKey, F17);
			new_enum_v(ImGuiKey, F18);
			new_enum_v(ImGuiKey, F19);
			new_enum_v(ImGuiKey, F20);
			new_enum_v(ImGuiKey, F21);
			new_enum_v(ImGuiKey, F22);
			new_enum_v(ImGuiKey, F23);
			new_enum_v(ImGuiKey, F24);
			new_enum_v(ImGuiKey, Apostrophe);
			new_enum_v(ImGuiKey, Comma);
			new_enum_v(ImGuiKey, Minus);
			new_enum_v(ImGuiKey, Period);
			new_enum_v(ImGuiKey, Slash);
			new_enum_v(ImGuiKey, Semicolon);
			new_enum_v(ImGuiKey, Equal);
			new_enum_v(ImGuiKey, LeftBracket);
			new_enum_v(ImGuiKey, Backslash);
			new_enum_v(ImGuiKey, RightBracket);
			new_enum_v(ImGuiKey, GraveAccent);
			new_enum_v(ImGuiKey, CapsLock);
			new_enum_v(ImGuiKey, ScrollLock);
			new_enum_v(ImGuiKey, NumLock);
			new_enum_v(ImGuiKey, PrintScreen);
			new_enum_v(ImGuiKey, Pause);
			new_enum_v(ImGuiKey, Keypad0);
			new_enum_v(ImGuiKey, Keypad1);
			new_enum_v(ImGuiKey, Keypad2);
			new_enum_v(ImGuiKey, Keypad3);
			new_enum_v(ImGuiKey, Keypad4);
			new_enum_v(ImGuiKey, Keypad5);
			new_enum_v(ImGuiKey, Keypad6);
			new_enum_v(ImGuiKey, Keypad7);
			new_enum_v(ImGuiKey, Keypad8);
			new_enum_v(ImGuiKey, Keypad9);
			new_enum_v(ImGuiKey, KeypadDecimal);
			new_enum_v(ImGuiKey, KeypadDivide);
			new_enum_v(ImGuiKey, KeypadMultiply);
			new_enum_v(ImGuiKey, KeypadSubtract);
			new_enum_v(ImGuiKey, KeypadAdd);
			new_enum_v(ImGuiKey, KeypadEnter);
			new_enum_v(ImGuiKey, KeypadEqual);
			new_enum_v(ImGuiKey, AppBack);
			new_enum_v(ImGuiKey, AppForward);
			new_enum_v(ImGuiKey, GamepadStart);
			new_enum_v(ImGuiKey, GamepadBack);
			new_enum_v(ImGuiKey, GamepadFaceLeft);
			new_enum_v(ImGuiKey, GamepadFaceRight);
			new_enum_v(ImGuiKey, GamepadFaceUp);
			new_enum_v(ImGuiKey, GamepadFaceDown);
			new_enum_v(ImGuiKey, GamepadDpadLeft);
			new_enum_v(ImGuiKey, GamepadDpadRight);
			new_enum_v(ImGuiKey, GamepadDpadUp);
			new_enum_v(ImGuiKey, GamepadDpadDown);
			new_enum_v(ImGuiKey, GamepadL1);
			new_enum_v(ImGuiKey, GamepadR1);
			new_enum_v(ImGuiKey, GamepadL2);
			new_enum_v(ImGuiKey, GamepadR2);
			new_enum_v(ImGuiKey, GamepadL3);
			new_enum_v(ImGuiKey, GamepadR3);
			new_enum_v(ImGuiKey, GamepadLStickLeft);
			new_enum_v(ImGuiKey, GamepadLStickRight);
			new_enum_v(ImGuiKey, GamepadLStickUp);
			new_enum_v(ImGuiKey, GamepadLStickDown);
			new_enum_v(ImGuiKey, GamepadRStickLeft);
			new_enum_v(ImGuiKey, GamepadRStickRight);
			new_enum_v(ImGuiKey, GamepadRStickUp);
			new_enum_v(ImGuiKey, GamepadRStickDown);
			new_enum_v(ImGuiKey, MouseLeft);
			new_enum_v(ImGuiKey, MouseRight);
			new_enum_v(ImGuiKey, MouseMiddle);
			new_enum_v(ImGuiKey, MouseX1);
			new_enum_v(ImGuiKey, MouseX2);
			new_enum_v(ImGuiKey, MouseWheelX);
			new_enum_v(ImGuiKey, MouseWheelY);
			new_enum_v(ImGuiKey, ReservedForModCtrl);
			new_enum_v(ImGuiKey, ReservedForModShift);
			new_enum_v(ImGuiKey, ReservedForModAlt);
			new_enum_v(ImGuiKey, ReservedForModSuper);
			new_enum_v(ImGuiKey, COUNT);
			new_enum.set("Mod_None", ImGuiMod_None);
			new_enum.set("Mod_Ctrl", ImGuiMod_Ctrl);
			new_enum.set("Mod_Shift", ImGuiMod_Shift);
			new_enum.set("Mod_Alt", ImGuiMod_Alt);
			new_enum.set("Mod_Super", ImGuiMod_Super);
			new_enum.set("Mod_Shortcut", ImGuiMod_Shortcut);
			new_enum.set("Mod_Mask_", ImGuiMod_Mask_);
			new_enum_v(ImGuiKey, NamedKey_BEGIN);
			new_enum_v(ImGuiKey, NamedKey_END);
			new_enum_v(ImGuiKey, NamedKey_COUNT);
			new_enum_v(ImGuiKey, ModCtrl);
			new_enum_v(ImGuiKey, ModShift);
			new_enum_v(ImGuiKey, ModAlt);
			new_enum_v(ImGuiKey, ModSuper);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiConfigFlags");
			new_enum_v(ImGuiConfigFlags, None);
			new_enum_v(ImGuiConfigFlags, NavEnableKeyboard);
			new_enum_v(ImGuiConfigFlags, NavEnableGamepad);
			new_enum_v(ImGuiConfigFlags, NavEnableSetMousePos);
			new_enum_v(ImGuiConfigFlags, NavNoCaptureKeyboard);
			new_enum_v(ImGuiConfigFlags, NoMouse);
			new_enum_v(ImGuiConfigFlags, NoMouseCursorChange);
			new_enum_v(ImGuiConfigFlags, DockingEnable);
			new_enum_v(ImGuiConfigFlags, ViewportsEnable);
			new_enum_v(ImGuiConfigFlags, DpiEnableScaleViewports);
			new_enum_v(ImGuiConfigFlags, DpiEnableScaleFonts);
			new_enum_v(ImGuiConfigFlags, IsSRGB);
			new_enum_v(ImGuiConfigFlags, IsTouchScreen);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiBackendFlags");
			new_enum_v(ImGuiBackendFlags, None);
			new_enum_v(ImGuiBackendFlags, HasGamepad);
			new_enum_v(ImGuiBackendFlags, HasMouseCursors);
			new_enum_v(ImGuiBackendFlags, HasSetMousePos);
			new_enum_v(ImGuiBackendFlags, RendererHasVtxOffset);
			new_enum_v(ImGuiBackendFlags, PlatformHasViewports);
			new_enum_v(ImGuiBackendFlags, HasMouseHoveredViewport);
			new_enum_v(ImGuiBackendFlags, RendererHasViewports);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiCol");
			new_enum_v(ImGuiCol, Text);
			new_enum_v(ImGuiCol, TextDisabled);
			new_enum_v(ImGuiCol, WindowBg);
			new_enum_v(ImGuiCol, ChildBg);
			new_enum_v(ImGuiCol, PopupBg);
			new_enum_v(ImGuiCol, Border);
			new_enum_v(ImGuiCol, BorderShadow);
			new_enum_v(ImGuiCol, FrameBg);
			new_enum_v(ImGuiCol, FrameBgHovered);
			new_enum_v(ImGuiCol, FrameBgActive);
			new_enum_v(ImGuiCol, TitleBg);
			new_enum_v(ImGuiCol, TitleBgActive);
			new_enum_v(ImGuiCol, TitleBgCollapsed);
			new_enum_v(ImGuiCol, MenuBarBg);
			new_enum_v(ImGuiCol, ScrollbarBg);
			new_enum_v(ImGuiCol, ScrollbarGrab);
			new_enum_v(ImGuiCol, ScrollbarGrabHovered);
			new_enum_v(ImGuiCol, ScrollbarGrabActive);
			new_enum_v(ImGuiCol, CheckMark);
			new_enum_v(ImGuiCol, SliderGrab);
			new_enum_v(ImGuiCol, SliderGrabActive);
			new_enum_v(ImGuiCol, Button);
			new_enum_v(ImGuiCol, ButtonHovered);
			new_enum_v(ImGuiCol, ButtonActive);
			new_enum_v(ImGuiCol, Header);
			new_enum_v(ImGuiCol, HeaderHovered);
			new_enum_v(ImGuiCol, HeaderActive);
			new_enum_v(ImGuiCol, Separator);
			new_enum_v(ImGuiCol, SeparatorHovered);
			new_enum_v(ImGuiCol, SeparatorActive);
			new_enum_v(ImGuiCol, ResizeGrip);
			new_enum_v(ImGuiCol, ResizeGripHovered);
			new_enum_v(ImGuiCol, ResizeGripActive);
			new_enum_v(ImGuiCol, Tab);
			new_enum_v(ImGuiCol, TabHovered);
			new_enum_v(ImGuiCol, TabActive);
			new_enum_v(ImGuiCol, TabUnfocused);
			new_enum_v(ImGuiCol, TabUnfocusedActive);
			new_enum_v(ImGuiCol, DockingPreview);
			new_enum_v(ImGuiCol, DockingEmptyBg);
			new_enum_v(ImGuiCol, PlotLines);
			new_enum_v(ImGuiCol, PlotLinesHovered);
			new_enum_v(ImGuiCol, PlotHistogram);
			new_enum_v(ImGuiCol, PlotHistogramHovered);
			new_enum_v(ImGuiCol, TableHeaderBg);
			new_enum_v(ImGuiCol, TableBorderStrong);
			new_enum_v(ImGuiCol, TableBorderLight);
			new_enum_v(ImGuiCol, TableRowBg);
			new_enum_v(ImGuiCol, TableRowBgAlt);
			new_enum_v(ImGuiCol, TextSelectedBg);
			new_enum_v(ImGuiCol, DragDropTarget);
			new_enum_v(ImGuiCol, NavHighlight);
			new_enum_v(ImGuiCol, NavWindowingHighlight);
			new_enum_v(ImGuiCol, NavWindowingDimBg);
			new_enum_v(ImGuiCol, ModalWindowDimBg);
			new_enum_v(ImGuiCol, COUNT);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiStyleVar");
			new_enum_v(ImGuiStyleVar, Alpha);
			new_enum_v(ImGuiStyleVar, DisabledAlpha);
			new_enum_v(ImGuiStyleVar, WindowPadding);
			new_enum_v(ImGuiStyleVar, WindowRounding);
			new_enum_v(ImGuiStyleVar, WindowBorderSize);
			new_enum_v(ImGuiStyleVar, WindowMinSize);
			new_enum_v(ImGuiStyleVar, WindowTitleAlign);
			new_enum_v(ImGuiStyleVar, ChildRounding);
			new_enum_v(ImGuiStyleVar, ChildBorderSize);
			new_enum_v(ImGuiStyleVar, PopupRounding);
			new_enum_v(ImGuiStyleVar, PopupBorderSize);
			new_enum_v(ImGuiStyleVar, FramePadding);
			new_enum_v(ImGuiStyleVar, FrameRounding);
			new_enum_v(ImGuiStyleVar, FrameBorderSize);
			new_enum_v(ImGuiStyleVar, ItemSpacing);
			new_enum_v(ImGuiStyleVar, ItemInnerSpacing);
			new_enum_v(ImGuiStyleVar, IndentSpacing);
			new_enum_v(ImGuiStyleVar, CellPadding);
			new_enum_v(ImGuiStyleVar, ScrollbarSize);
			new_enum_v(ImGuiStyleVar, ScrollbarRounding);
			new_enum_v(ImGuiStyleVar, GrabMinSize);
			new_enum_v(ImGuiStyleVar, GrabRounding);
			new_enum_v(ImGuiStyleVar, TabRounding);
			new_enum_v(ImGuiStyleVar, TabBarBorderSize);
			new_enum_v(ImGuiStyleVar, ButtonTextAlign);
			new_enum_v(ImGuiStyleVar, SelectableTextAlign);
			new_enum_v(ImGuiStyleVar, SeparatorTextBorderSize);
			new_enum_v(ImGuiStyleVar, SeparatorTextAlign);
			new_enum_v(ImGuiStyleVar, SeparatorTextPadding);
			new_enum_v(ImGuiStyleVar, DockingSeparatorSize);
			new_enum_v(ImGuiStyleVar, LayoutAlign);
			new_enum_v(ImGuiStyleVar, COUNT);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiButtonFlags");
			new_enum_v(ImGuiButtonFlags, None);
			new_enum_v(ImGuiButtonFlags, MouseButtonLeft);
			new_enum_v(ImGuiButtonFlags, MouseButtonRight);
			new_enum_v(ImGuiButtonFlags, MouseButtonMiddle);
			new_enum_v(ImGuiButtonFlags, MouseButtonMask_);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiColorEditFlags");
			new_enum_v(ImGuiColorEditFlags, None);
			new_enum_v(ImGuiColorEditFlags, NoAlpha);
			new_enum_v(ImGuiColorEditFlags, NoPicker);
			new_enum_v(ImGuiColorEditFlags, NoOptions);
			new_enum_v(ImGuiColorEditFlags, NoSmallPreview);
			new_enum_v(ImGuiColorEditFlags, NoInputs);
			new_enum_v(ImGuiColorEditFlags, NoTooltip);
			new_enum_v(ImGuiColorEditFlags, NoLabel);
			new_enum_v(ImGuiColorEditFlags, NoSidePreview);
			new_enum_v(ImGuiColorEditFlags, NoDragDrop);
			new_enum_v(ImGuiColorEditFlags, NoBorder);
			new_enum_v(ImGuiColorEditFlags, AlphaBar);
			new_enum_v(ImGuiColorEditFlags, AlphaPreview);
			new_enum_v(ImGuiColorEditFlags, AlphaPreviewHalf);
			new_enum_v(ImGuiColorEditFlags, HDR);
			new_enum_v(ImGuiColorEditFlags, DisplayRGB);
			new_enum_v(ImGuiColorEditFlags, DisplayHSV);
			new_enum_v(ImGuiColorEditFlags, DisplayHex);
			new_enum_v(ImGuiColorEditFlags, Uint8);
			new_enum_v(ImGuiColorEditFlags, Float);
			new_enum_v(ImGuiColorEditFlags, PickerHueBar);
			new_enum_v(ImGuiColorEditFlags, PickerHueWheel);
			new_enum_v(ImGuiColorEditFlags, InputRGB);
			new_enum_v(ImGuiColorEditFlags, InputHSV);
			new_enum_v(ImGuiColorEditFlags, DefaultOptions_);
			new_enum_v(ImGuiColorEditFlags, DisplayMask_);
			new_enum_v(ImGuiColorEditFlags, DataTypeMask_);
			new_enum_v(ImGuiColorEditFlags, PickerMask_);
			new_enum_v(ImGuiColorEditFlags, InputMask_);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiSliderFlags");
			new_enum_v(ImGuiSliderFlags, None);
			new_enum_v(ImGuiSliderFlags, AlwaysClamp);
			new_enum_v(ImGuiSliderFlags, Logarithmic);
			new_enum_v(ImGuiSliderFlags, NoRoundToFormat);
			new_enum_v(ImGuiSliderFlags, NoInput);
			new_enum_v(ImGuiSliderFlags, InvalidMask_);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiMouseButton");
			new_enum_v(ImGuiMouseButton, Left);
			new_enum_v(ImGuiMouseButton, Right);
			new_enum_v(ImGuiMouseButton, Middle);
			new_enum_v(ImGuiMouseButton, COUNT);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiMouseCursor");
			new_enum_v(ImGuiMouseCursor, None);
			new_enum_v(ImGuiMouseCursor, Arrow);
			new_enum_v(ImGuiMouseCursor, TextInput);
			new_enum_v(ImGuiMouseCursor, ResizeAll);
			new_enum_v(ImGuiMouseCursor, ResizeNS);
			new_enum_v(ImGuiMouseCursor, ResizeEW);
			new_enum_v(ImGuiMouseCursor, ResizeNESW);
			new_enum_v(ImGuiMouseCursor, ResizeNWSE);
			new_enum_v(ImGuiMouseCursor, Hand);
			new_enum_v(ImGuiMouseCursor, NotAllowed);
			new_enum_v(ImGuiMouseCursor, COUNT);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiMouseSource");
			new_enum_v(ImGuiMouseSource, Mouse);
			new_enum_v(ImGuiMouseSource, TouchScreen);
			new_enum_v(ImGuiMouseSource, Pen);
			new_enum_v(ImGuiMouseSource, COUNT);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiCond");
			new_enum_v(ImGuiCond, None);
			new_enum_v(ImGuiCond, Always);
			new_enum_v(ImGuiCond, Once);
			new_enum_v(ImGuiCond, FirstUseEver);
			new_enum_v(ImGuiCond, Appearing);
		}
		{
			ScriptEnumRegistrar new_enum("ImDrawFlags");
			new_enum_v(ImDrawFlags, None);
			new_enum_v(ImDrawFlags, Closed);
			new_enum_v(ImDrawFlags, RoundCornersTopLeft);
			new_enum_v(ImDrawFlags, RoundCornersTopRight);
			new_enum_v(ImDrawFlags, RoundCornersBottomLeft);
			new_enum_v(ImDrawFlags, RoundCornersBottomRight);
			new_enum_v(ImDrawFlags, RoundCornersNone);
			new_enum_v(ImDrawFlags, RoundCornersTop);
			new_enum_v(ImDrawFlags, RoundCornersBottom);
			new_enum_v(ImDrawFlags, RoundCornersLeft);
			new_enum_v(ImDrawFlags, RoundCornersRight);
			new_enum_v(ImDrawFlags, RoundCornersAll);
			new_enum_v(ImDrawFlags, RoundCornersDefault_);
			new_enum_v(ImDrawFlags, RoundCornersMask_);
		}
		{
			ScriptEnumRegistrar new_enum("ImDrawListFlags");
			new_enum_v(ImDrawListFlags, None);
			new_enum_v(ImDrawListFlags, AntiAliasedLines);
			new_enum_v(ImDrawListFlags, AntiAliasedLinesUseTex);
			new_enum_v(ImDrawListFlags, AntiAliasedFill);
			new_enum_v(ImDrawListFlags, AllowVtxOffset);
		}
		{
			ScriptEnumRegistrar new_enum("ImFontAtlasFlags");
			new_enum_v(ImFontAtlasFlags, None);
			new_enum_v(ImFontAtlasFlags, NoPowerOfTwoHeight);
			new_enum_v(ImFontAtlasFlags, NoMouseCursors);
			new_enum_v(ImFontAtlasFlags, NoBakedLines);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiViewportFlags");
			new_enum_v(ImGuiViewportFlags, None);
			new_enum_v(ImGuiViewportFlags, IsPlatformWindow);
			new_enum_v(ImGuiViewportFlags, IsPlatformMonitor);
			new_enum_v(ImGuiViewportFlags, OwnedByApp);
			new_enum_v(ImGuiViewportFlags, NoDecoration);
			new_enum_v(ImGuiViewportFlags, NoTaskBarIcon);
			new_enum_v(ImGuiViewportFlags, NoFocusOnAppearing);
			new_enum_v(ImGuiViewportFlags, NoFocusOnClick);
			new_enum_v(ImGuiViewportFlags, NoInputs);
			new_enum_v(ImGuiViewportFlags, NoRendererClear);
			new_enum_v(ImGuiViewportFlags, NoAutoMerge);
			new_enum_v(ImGuiViewportFlags, TopMost);
			new_enum_v(ImGuiViewportFlags, CanHostOtherWindows);
			new_enum_v(ImGuiViewportFlags, IsMinimized);
			new_enum_v(ImGuiViewportFlags, IsFocused);
		}

		// imgui_internal.h
		{
			ScriptEnumRegistrar new_enum("ImGuiItemFlags");
			new_enum_v(ImGuiItemFlags, None);
			new_enum_v(ImGuiItemFlags, NoTabStop);
			new_enum_v(ImGuiItemFlags, ButtonRepeat);
			new_enum_v(ImGuiItemFlags, Disabled);
			new_enum_v(ImGuiItemFlags, NoNav);
			new_enum_v(ImGuiItemFlags, NoNavDefaultFocus);
			new_enum_v(ImGuiItemFlags, MixedValue);
			new_enum_v(ImGuiItemFlags, ReadOnly);
			new_enum_v(ImGuiItemFlags, NoWindowHoverableCheck);
			new_enum_v(ImGuiItemFlags, AllowOverlap);
			new_enum_v(ImGuiItemFlags, Inputable);
			new_enum_v(ImGuiItemFlags, HasSelectionUserData);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiItemStatusFlags");
			new_enum_v(ImGuiItemStatusFlags, None);
			new_enum_v(ImGuiItemStatusFlags, HoveredRect);
			new_enum_v(ImGuiItemStatusFlags, HasDisplayRect);
			new_enum_v(ImGuiItemStatusFlags, Edited);
			new_enum_v(ImGuiItemStatusFlags, ToggledSelection);
			new_enum_v(ImGuiItemStatusFlags, ToggledOpen);
			new_enum_v(ImGuiItemStatusFlags, HasDeactivated);
			new_enum_v(ImGuiItemStatusFlags, Deactivated);
			new_enum_v(ImGuiItemStatusFlags, HoveredWindow);
			new_enum_v(ImGuiItemStatusFlags, Visible);
			new_enum_v(ImGuiItemStatusFlags, HasClipRect);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiSeparatorFlags");
			new_enum_v(ImGuiSeparatorFlags, None);
			new_enum_v(ImGuiSeparatorFlags, Horizontal);
			new_enum_v(ImGuiSeparatorFlags, Vertical);
			new_enum_v(ImGuiSeparatorFlags, SpanAllColumns);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiFocusRequestFlags");
			new_enum_v(ImGuiFocusRequestFlags, None);
			new_enum_v(ImGuiFocusRequestFlags, RestoreFocusedChild);
			new_enum_v(ImGuiFocusRequestFlags, UnlessBelowModal);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiTextFlags");
			new_enum_v(ImGuiTextFlags, None);
			new_enum_v(ImGuiTextFlags, NoWidthForLargeClippedText);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiTooltipFlags");
			new_enum_v(ImGuiTooltipFlags, None);
			new_enum_v(ImGuiTooltipFlags, OverridePrevious);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiLayoutType");
			new_enum_v(ImGuiLayoutType, Horizontal);
			new_enum_v(ImGuiLayoutType, Vertical);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiLayoutItemType");
			new_enum_v(ImGuiLayoutItemType, Item);
			new_enum_v(ImGuiLayoutItemType, Spring);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiAxis");
			new_enum_v(ImGuiAxis, None);
			new_enum_v(ImGuiAxis, X);
			new_enum_v(ImGuiAxis, Y);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiPlotType");
			new_enum_v(ImGuiPlotType, Lines);
			new_enum_v(ImGuiPlotType, Histogram);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiPopupPositionPolicy");
			new_enum_v(ImGuiPopupPositionPolicy, Default);
			new_enum_v(ImGuiPopupPositionPolicy, ComboBox);
			new_enum_v(ImGuiPopupPositionPolicy, Tooltip);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiNextWindowDataFlags");
			new_enum_v(ImGuiNextWindowDataFlags, None);
			new_enum_v(ImGuiNextWindowDataFlags, HasPos);
			new_enum_v(ImGuiNextWindowDataFlags, HasSize);
			new_enum_v(ImGuiNextWindowDataFlags, HasContentSize);
			new_enum_v(ImGuiNextWindowDataFlags, HasCollapsed);
			new_enum_v(ImGuiNextWindowDataFlags, HasSizeConstraint);
			new_enum_v(ImGuiNextWindowDataFlags, HasFocus);
			new_enum_v(ImGuiNextWindowDataFlags, HasBgAlpha);
			new_enum_v(ImGuiNextWindowDataFlags, HasScroll);
			new_enum_v(ImGuiNextWindowDataFlags, HasChildFlags);
			new_enum_v(ImGuiNextWindowDataFlags, HasViewport);
			new_enum_v(ImGuiNextWindowDataFlags, HasDock);
			new_enum_v(ImGuiNextWindowDataFlags, HasWindowClass);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiNextItemDataFlags");
			new_enum_v(ImGuiNextItemDataFlags, None);
			new_enum_v(ImGuiNextItemDataFlags, HasWidth);
			new_enum_v(ImGuiNextItemDataFlags, HasOpen);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiInputEventType");
			new_enum_v(ImGuiInputEventType, None);
			new_enum_v(ImGuiInputEventType, MousePos);
			new_enum_v(ImGuiInputEventType, MouseWheel);
			new_enum_v(ImGuiInputEventType, MouseButton);
			new_enum_v(ImGuiInputEventType, MouseViewport);
			new_enum_v(ImGuiInputEventType, Key);
			new_enum_v(ImGuiInputEventType, Text);
			new_enum_v(ImGuiInputEventType, Focus);
			new_enum_v(ImGuiInputEventType, COUNT);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiInputSource");
			new_enum_v(ImGuiInputSource, None);
			new_enum_v(ImGuiInputSource, Mouse);
			new_enum_v(ImGuiInputSource, Keyboard);
			new_enum_v(ImGuiInputSource, Gamepad);
			new_enum_v(ImGuiInputSource, COUNT);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiInputFlags");
			new_enum_v(ImGuiInputFlags, None);
			new_enum_v(ImGuiInputFlags, Repeat);
			new_enum_v(ImGuiInputFlags, RepeatRateDefault);
			new_enum_v(ImGuiInputFlags, RepeatRateNavMove);
			new_enum_v(ImGuiInputFlags, RepeatRateNavTweak);
			new_enum_v(ImGuiInputFlags, RepeatRateMask_);
			new_enum_v(ImGuiInputFlags, CondHovered);
			new_enum_v(ImGuiInputFlags, CondActive);
			new_enum_v(ImGuiInputFlags, CondDefault_);
			new_enum_v(ImGuiInputFlags, CondMask_);
			new_enum_v(ImGuiInputFlags, LockThisFrame);
			new_enum_v(ImGuiInputFlags, LockUntilRelease);
			new_enum_v(ImGuiInputFlags, RouteFocused);
			new_enum_v(ImGuiInputFlags, RouteGlobal);
			new_enum_v(ImGuiInputFlags, RouteAlways);
			new_enum_v(ImGuiInputFlags, RouteUnlessBgFocused);
			new_enum_v(ImGuiInputFlags, SupportedByIsKeyPressed);
			new_enum_v(ImGuiInputFlags, SupportedByShortcut);
			new_enum_v(ImGuiInputFlags, SupportedBySetKeyOwner);
			new_enum_v(ImGuiInputFlags, SupportedBySetItemKeyOwner);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiActivateFlags");
			new_enum_v(ImGuiActivateFlags, None);
			new_enum_v(ImGuiActivateFlags, PreferInput);
			new_enum_v(ImGuiActivateFlags, PreferTweak);
			new_enum_v(ImGuiActivateFlags, TryToPreserveState);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiScrollFlags");
			new_enum_v(ImGuiScrollFlags, None);
			new_enum_v(ImGuiScrollFlags, KeepVisibleEdgeX);
			new_enum_v(ImGuiScrollFlags, KeepVisibleEdgeY);
			new_enum_v(ImGuiScrollFlags, KeepVisibleCenterX);
			new_enum_v(ImGuiScrollFlags, KeepVisibleCenterY);
			new_enum_v(ImGuiScrollFlags, AlwaysCenterX);
			new_enum_v(ImGuiScrollFlags, AlwaysCenterY);
			new_enum_v(ImGuiScrollFlags, NoScrollParent);
			new_enum_v(ImGuiScrollFlags, MaskX_);
			new_enum_v(ImGuiScrollFlags, MaskY_);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiNavHighlightFlags");
			new_enum_v(ImGuiNavHighlightFlags, None);
			new_enum_v(ImGuiNavHighlightFlags, AlwaysDraw);
			new_enum_v(ImGuiNavHighlightFlags, NoRounding);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiNavMoveFlags");
			new_enum_v(ImGuiNavMoveFlags, None);
			new_enum_v(ImGuiNavMoveFlags, LoopX);
			new_enum_v(ImGuiNavMoveFlags, LoopY);
			new_enum_v(ImGuiNavMoveFlags, WrapX);
			new_enum_v(ImGuiNavMoveFlags, WrapY);
			new_enum_v(ImGuiNavMoveFlags, WrapMask_);
			new_enum_v(ImGuiNavMoveFlags, AllowCurrentNavId);
			new_enum_v(ImGuiNavMoveFlags, AlsoScoreVisibleSet);
			new_enum_v(ImGuiNavMoveFlags, ScrollToEdgeY);
			new_enum_v(ImGuiNavMoveFlags, Forwarded);
			new_enum_v(ImGuiNavMoveFlags, DebugNoResult);
			new_enum_v(ImGuiNavMoveFlags, FocusApi);
			new_enum_v(ImGuiNavMoveFlags, IsTabbing);
			new_enum_v(ImGuiNavMoveFlags, IsPageMove);
			new_enum_v(ImGuiNavMoveFlags, Activate);
			new_enum_v(ImGuiNavMoveFlags, NoSelect);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiNavLayer");
			new_enum_v(ImGuiNavLayer, Main);
			new_enum_v(ImGuiNavLayer, Menu);
			new_enum_v(ImGuiNavLayer, COUNT);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiTypingSelectFlags");
			new_enum_v(ImGuiTypingSelectFlags, None);
			new_enum_v(ImGuiTypingSelectFlags, AllowBackspace);
			new_enum_v(ImGuiTypingSelectFlags, AllowSingleCharMode);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiDataAuthority");
			new_enum_v(ImGuiDataAuthority, Auto);
			new_enum_v(ImGuiDataAuthority, DockNode);
			new_enum_v(ImGuiDataAuthority, Window);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiDockNodeState");
			new_enum_v(ImGuiDockNodeState, Unknown);
			new_enum_v(ImGuiDockNodeState, HostWindowHiddenBecauseSingleWindow);
			new_enum_v(ImGuiDockNodeState, HostWindowHiddenBecauseWindowsAreResizing);
			new_enum_v(ImGuiDockNodeState, HostWindowVisible);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiWindowDockStyleCol");
			new_enum_v(ImGuiWindowDockStyleCol, Text);
			new_enum_v(ImGuiWindowDockStyleCol, TabHovered);
			new_enum_v(ImGuiWindowDockStyleCol, COUNT);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiLocKey");
			new_enum_v(ImGuiLocKey, VersionStr);
			new_enum_v(ImGuiLocKey, TableSizeOne);
			new_enum_v(ImGuiLocKey, TableSizeAllFit);
			new_enum_v(ImGuiLocKey, TableSizeAllDefault);
			new_enum_v(ImGuiLocKey, TableResetOrder);
			new_enum_v(ImGuiLocKey, WindowingMainMenuBar);
			new_enum_v(ImGuiLocKey, WindowingPopup);
			new_enum_v(ImGuiLocKey, WindowingUntitled);
			new_enum_v(ImGuiLocKey, DockingHideTabBar);
			new_enum_v(ImGuiLocKey, DockingHoldShiftToDock);
			new_enum_v(ImGuiLocKey, DockingDragToUndockOrMoveNode);
			new_enum_v(ImGuiLocKey, COUNT);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiDebugLogFlags");
			new_enum_v(ImGuiDebugLogFlags, None);
			new_enum_v(ImGuiDebugLogFlags, EventActiveId);
			new_enum_v(ImGuiDebugLogFlags, EventFocus);
			new_enum_v(ImGuiDebugLogFlags, EventPopup);
			new_enum_v(ImGuiDebugLogFlags, EventNav);
			new_enum_v(ImGuiDebugLogFlags, EventClipper);
			new_enum_v(ImGuiDebugLogFlags, EventSelection);
			new_enum_v(ImGuiDebugLogFlags, EventIO);
			new_enum_v(ImGuiDebugLogFlags, EventDocking);
			new_enum_v(ImGuiDebugLogFlags, EventViewport);
			new_enum_v(ImGuiDebugLogFlags, EventMask_);
			new_enum_v(ImGuiDebugLogFlags, OutputToTTY);
			new_enum_v(ImGuiDebugLogFlags, OutputToTestEngine);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiContextHookType");
			new_enum_v(ImGuiContextHookType, NewFramePre);
			new_enum_v(ImGuiContextHookType, NewFramePost);
			new_enum_v(ImGuiContextHookType, EndFramePre);
			new_enum_v(ImGuiContextHookType, EndFramePost);
			new_enum_v(ImGuiContextHookType, RenderPre);
			new_enum_v(ImGuiContextHookType, RenderPost);
			new_enum_v(ImGuiContextHookType, Shutdown);
			new_enum_v(ImGuiContextHookType, BeginWindow);
			new_enum_v(ImGuiContextHookType, EndWindow);
			new_enum_v(ImGuiContextHookType, PendingRemoval_);
		}
	}

	static void register_structures()
	{
		{
			auto info              = ScriptClassRegistrar::ValueInfo::from<ImVec2>();
			info.all_floats        = true;
			info.pod               = true;
			ScriptClassRegistrar r = ScriptClassRegistrar::value_class("ImVec2", sizeof(ImVec2), info);
			r.behave(ScriptClassBehave::Construct, "void f()", r.constructor<ImVec2>);
			r.behave(ScriptClassBehave::Construct, "void f(float x, float y)", r.constructor<ImVec2, float, float>);
			r.property("float x", &ImVec2::x);
			r.property("float y", &ImVec2::y);
		}

		{
			auto info              = ScriptClassRegistrar::ValueInfo::from<ImVec3>();
			info.all_floats        = true;
			info.pod               = true;
			ScriptClassRegistrar r = ScriptClassRegistrar::value_class("ImVec3", sizeof(ImVec3), info);
			r.behave(ScriptClassBehave::Construct, "void f()", r.constructor<ImVec3>);
			r.behave(ScriptClassBehave::Construct, "void f(float x, float y, float z)",
			         r.constructor<ImVec3, float, float, float>);
			r.property("float x", &ImVec3::x);
			r.property("float y", &ImVec3::y);
			r.property("float z", &ImVec3::z);
		}

		{
			auto info              = ScriptClassRegistrar::ValueInfo::from<ImVec4>();
			info.all_floats        = true;
			info.pod               = true;
			ScriptClassRegistrar r = ScriptClassRegistrar::value_class("ImVec4", sizeof(ImVec4), info);
			r.behave(ScriptClassBehave::Construct, "void f()", r.constructor<ImVec4>);
			r.behave(ScriptClassBehave::Construct, "void f(float x, float y, float z, float w)",
			         r.constructor<ImVec4, float, float, float, float>);
			r.property("float x", &ImVec4::x);
			r.property("float y", &ImVec4::y);
			r.property("float z", &ImVec4::z);
			r.property("float w", &ImVec4::w);
		}
		{
			auto info              = ScriptClassRegistrar::ValueInfo::from<ImIVec2>();
			info.all_ints          = true;
			info.pod               = true;
			ScriptClassRegistrar r = ScriptClassRegistrar::value_class("ImIVec2", sizeof(ImIVec2), info);
			r.behave(ScriptClassBehave::Construct, "void f()", r.constructor<ImIVec2>);
			r.behave(ScriptClassBehave::Construct, "void f(int x, int y)", r.constructor<ImIVec2, int, int>);
			r.property("int x", &ImIVec2::x);
			r.property("int y", &ImIVec2::y);
		}

		{
			auto info              = ScriptClassRegistrar::ValueInfo::from<ImIVec3>();
			info.all_ints          = true;
			info.pod               = true;
			ScriptClassRegistrar r = ScriptClassRegistrar::value_class("ImIVec3", sizeof(ImIVec3), info);
			r.behave(ScriptClassBehave::Construct, "void f()", r.constructor<ImIVec3>);
			r.behave(ScriptClassBehave::Construct, "void f(int x, int y, int z)", r.constructor<ImIVec3, int, int, int>);
			r.property("int x", &ImIVec3::x);
			r.property("int y", &ImIVec3::y);
			r.property("int z", &ImIVec3::z);
		}

		{
			auto info              = ScriptClassRegistrar::ValueInfo::from<ImIVec4>();
			info.all_ints          = true;
			info.pod               = true;
			ScriptClassRegistrar r = ScriptClassRegistrar::value_class("ImIVec4", sizeof(ImIVec4), info);
			r.behave(ScriptClassBehave::Construct, "void f()", r.constructor<ImIVec4>);
			r.behave(ScriptClassBehave::Construct, "void f(int x, int y, int z, int w)",
			         r.constructor<ImIVec4, int, int, int, int>);
			r.property("int x", &ImIVec4::x);
			r.property("int y", &ImIVec4::y);
			r.property("int z", &ImIVec4::z);
			r.property("int w", &ImIVec4::w);
		}
		{
			auto info              = ScriptClassRegistrar::ValueInfo::from<ImGuiStyle>();
			info.pod               = true;
			ScriptClassRegistrar r = ScriptClassRegistrar::value_class("ImGuiStyle", sizeof(ImGuiStyle), info);
			r.behave(ScriptClassBehave::Construct, "void f()", r.constructor<ImGuiStyle>);
			r.method("void ScaleAllSizes(float scale_factor)", &ImGuiStyle::ScaleAllSizes);

			auto get_color_proxy = static_cast<const ImVec4& (*) (ImGuiStyle*, int)>(
			        [](ImGuiStyle* self, int idx) -> const ImVec4& { return self->Colors[idx]; });
			auto set_color_proxy = static_cast<void (*)(ImGuiStyle*, int, const ImVec4& value)>(
			        [](ImGuiStyle* self, int idx, const ImVec4& color) { self->Colors[idx] = color; });

			r.property("float Alpha", &ImGuiStyle::Alpha);
			r.property("float DisabledAlpha", &ImGuiStyle::DisabledAlpha);
			r.property("ImVec2 WindowPadding", &ImGuiStyle::WindowPadding);
			r.property("float WindowRounding", &ImGuiStyle::WindowRounding);
			r.property("float WindowBorderSize", &ImGuiStyle::WindowBorderSize);
			r.property("ImVec2 WindowMinSize", &ImGuiStyle::WindowMinSize);
			r.property("ImVec2 WindowTitleAlign", &ImGuiStyle::WindowTitleAlign);
			r.property("ImGuiDir WindowMenuButtonPosition", &ImGuiStyle::WindowMenuButtonPosition);
			r.property("float ChildRounding", &ImGuiStyle::ChildRounding);
			r.property("float ChildBorderSize", &ImGuiStyle::ChildBorderSize);
			r.property("float PopupRounding", &ImGuiStyle::PopupRounding);
			r.property("float PopupBorderSize", &ImGuiStyle::PopupBorderSize);
			r.property("ImVec2 FramePadding", &ImGuiStyle::FramePadding);
			r.property("float FrameRounding", &ImGuiStyle::FrameRounding);
			r.property("float FrameBorderSize", &ImGuiStyle::FrameBorderSize);
			r.property("ImVec2 ItemSpacing", &ImGuiStyle::ItemSpacing);
			r.property("ImVec2 ItemInnerSpacing", &ImGuiStyle::ItemInnerSpacing);
			r.property("ImVec2 CellPadding", &ImGuiStyle::CellPadding);
			r.property("ImVec2 TouchExtraPadding", &ImGuiStyle::TouchExtraPadding);
			r.property("float IndentSpacing", &ImGuiStyle::IndentSpacing);
			r.property("float ColumnsMinSpacing", &ImGuiStyle::ColumnsMinSpacing);
			r.property("float ScrollbarSize", &ImGuiStyle::ScrollbarSize);
			r.property("float ScrollbarRounding", &ImGuiStyle::ScrollbarRounding);
			r.property("float GrabMinSize", &ImGuiStyle::GrabMinSize);
			r.property("float GrabRounding", &ImGuiStyle::GrabRounding);
			r.property("float LayoutAlign", &ImGuiStyle::LayoutAlign);
			r.property("float LogSliderDeadzone", &ImGuiStyle::LogSliderDeadzone);
			r.property("float TabRounding", &ImGuiStyle::TabRounding);
			r.property("float TabBorderSize", &ImGuiStyle::TabBorderSize);
			r.property("float TabCloseButtonMinWidthSelected", &ImGuiStyle::TabCloseButtonMinWidthSelected);
			r.property("float TabCloseButtonMinWidthUnselected", &ImGuiStyle::TabCloseButtonMinWidthUnselected);
			r.property("float TabBarBorderSize", &ImGuiStyle::TabBarBorderSize);
			r.property("float TableAngledHeadersAngle", &ImGuiStyle::TableAngledHeadersAngle);
			r.property("ImGuiDir ColorButtonPosition", &ImGuiStyle::ColorButtonPosition);
			r.property("ImVec2 ButtonTextAlign", &ImGuiStyle::ButtonTextAlign);
			r.property("ImVec2 SelectableTextAlign", &ImGuiStyle::SelectableTextAlign);
			r.property("float SeparatorTextBorderSize", &ImGuiStyle::SeparatorTextBorderSize);
			r.property("ImVec2 SeparatorTextAlign", &ImGuiStyle::SeparatorTextAlign);
			r.property("ImVec2 SeparatorTextPadding", &ImGuiStyle::SeparatorTextPadding);
			r.property("ImVec2 DisplayWindowPadding", &ImGuiStyle::DisplayWindowPadding);
			r.property("ImVec2 DisplaySafeAreaPadding", &ImGuiStyle::DisplaySafeAreaPadding);
			r.property("float DockingSeparatorSize", &ImGuiStyle::DockingSeparatorSize);
			r.property("float MouseCursorScale", &ImGuiStyle::MouseCursorScale);
			r.property("bool AntiAliasedLines", &ImGuiStyle::AntiAliasedLines);
			r.property("bool AntiAliasedLinesUseTex", &ImGuiStyle::AntiAliasedLinesUseTex);
			r.property("bool AntiAliasedFill", &ImGuiStyle::AntiAliasedFill);
			r.property("float CurveTessellationTol", &ImGuiStyle::CurveTessellationTol);
			r.property("float CircleTessellationMaxError", &ImGuiStyle::CircleTessellationMaxError);
			r.method("const ImVec4& get_Colors(int color) property", get_color_proxy);
			r.method("void set_Colors(int color, const ImVec4& color) property", set_color_proxy);
			r.property("float HoverStationaryDelay", &ImGuiStyle::HoverStationaryDelay);
			r.property("float HoverDelayShort", &ImGuiStyle::HoverDelayShort);
			r.property("float HoverDelayNormal", &ImGuiStyle::HoverDelayNormal);
			r.property("ImGuiHoveredFlags HoverFlagsForTooltipMouse", &ImGuiStyle::HoverFlagsForTooltipMouse);
			r.property("ImGuiHoveredFlags HoverFlagsForTooltipNav", &ImGuiStyle::HoverFlagsForTooltipNav);
		}
	}

	static void register_functions()
	{
		ScriptNamespaceScopedChanger imgui("ImGui");
		auto& e = ScriptEngine::instance();

		struct Custom {
			static void Text(const String& str) { ImGui::Text("%s", str.c_str()); }

			static void TextColored(const ImVec4& col, const String& str) { ImGui::TextColored(col, "%s", str.c_str()); }

			static void TextDisabled(const String& label) { ImGui::TextDisabled("%s", label.c_str()); }

			static void TextWrapped(const String& label) { ImGui::TextWrapped("%s", label.c_str()); }

			static void LabelText(const String& str, const String& label) { ImGui::LabelText(str.c_str(), "%s", label.c_str()); }

			static void BulletText(const String& label) { ImGui::BulletText("%s", label.c_str()); }

			static bool Checkbox(const String& text, bool& value) { return ImGui::Checkbox(text.c_str(), &value); }

			static bool CheckboxFlags(const String& text, uint64_t& flags, uint64_t value)
			{
				return ImGui::CheckboxFlags(text.c_str(), reinterpret_cast<ImU64*>(&flags), static_cast<ImU64>(value));
			}

			static bool RadioButton(const String& label, int& value, int v_button)
			{
				return ImGui::RadioButton(label.c_str(), &value, v_button);
			}

			static bool Combo(const String& label, int& current_item, const Vector<String>& items, int height_in_items)
			{
				StackByteAllocator::Mark mark;

				const size_t count = items.size();
				const char** data  = StackAllocator<const char*>::allocate(count);

				etl::transform(items.begin(), items.end(), data, [](const String& item) -> const char* { return item.c_str(); });
				return ImGui::Combo(label.c_str(), &current_item, data, count, height_in_items);
			}

			static bool Combo2(const String& label, int& current_item, const String& items_separated_by_zeros,
			                   int height_in_items)
			{
				return ImGui::Combo(label.c_str(), &current_item, items_separated_by_zeros.c_str(), height_in_items);
			}

			static bool InputText(const String& label, String& buffer, int flags)
			{
				return ImGui::InputText(label.c_str(), buffer, flags);
			}

			static bool InputTextMultiline(const String& label, String& buffer, const ImVec2& size, int flags)
			{
				return ImGui::InputTextMultiline(label.c_str(), buffer, size, flags);
			}

			static bool InputTextWithHint(const String& label, const String& hint, String& buffer, int flags)
			{
				return ImGui::InputTextWithHint(label.c_str(), hint.c_str(), buffer, flags);
			}

			static bool TreeNode(ScriptPointer ptr, const String& text)
			{
				return ImGui::TreeNode(ptr.address(), "%s", text.c_str());
			}

			static bool TreeNodeEx(const String& str_id, const String& text, int flags)
			{
				return ImGui::TreeNodeEx(str_id.c_str(), flags, "%s", text.c_str());
			}

			static bool TreeNodeEx2(ScriptPointer ptr, const String& text, int flags)
			{
				return ImGui::TreeNodeEx(ptr.address(), flags, "%s", text.c_str());
			}
		};

		// clang-format off
		e.register_function("void ShowDemoWindow(Ptr<bool> p_open = nullptr)", ImBinder<&ImGui::ShowDemoWindow>::bind());
		e.register_function("void ShowMetricsWindow(Ptr<bool> p_open = nullptr)", ImBinder<&ImGui::ShowMetricsWindow>::bind());
		e.register_function("void ShowDebugLogWindow(Ptr<bool> p_open = nullptr)", ImBinder<&ImGui::ShowDebugLogWindow>::bind());
		e.register_function("void ShowIDStackToolWindow(Ptr<bool> p_open = nullptr)", ImBinder<&ImGui::ShowIDStackToolWindow>::bind());
		e.register_function("void ShowAboutWindow(Ptr<bool> p_open = nullptr)", ImBinder<overload_of<void(bool *)>(&ImGui::ShowAboutWindow)>::bind());
		e.register_function("void ShowStyleEditor(Ptr<ImGuiStyle> ref = nullptr)", ImBinder<overload_of<void(ImGuiStyle *)>(&ImGui::ShowStyleEditor)>::bind());
		e.register_function("bool ShowStyleSelector(const string& label)", ImBinder<overload_of<bool(const char *)>(&ImGui::ShowStyleSelector)>::bind());
		e.register_function("void ShowFontSelector(const string& label)", ImBinder<overload_of<void(const char *)>(&ImGui::ShowFontSelector)>::bind());
		e.register_function("void ShowUserGuide()", ImBinder<overload_of<void()>(&ImGui::ShowUserGuide)>::bind());
		e.register_function("string GetVersion()", ImBinder<overload_of<const char *()>(&ImGui::GetVersion)>::bind());
		e.register_function("void StyleColorsDark(Ptr<ImGuiStyle> dst = nullptr)", ImBinder<overload_of<void(ImGuiStyle *)>(&ImGui::StyleColorsDark)>::bind());
		e.register_function("void StyleColorsLight(Ptr<ImGuiStyle> dst = nullptr)", ImBinder<overload_of<void(ImGuiStyle *)>(&ImGui::StyleColorsLight)>::bind());
		e.register_function("void StyleColorsClassic(Ptr<ImGuiStyle> dst = nullptr)", ImBinder<overload_of<void(ImGuiStyle *)>(&ImGui::StyleColorsClassic)>::bind());
		e.register_function("bool Begin(const string& name, Ptr<bool> p_open = nullptr, int flags = 0)", ImBinder<overload_of<bool(const char *, bool *, ImGuiWindowFlags)>(&ImGui::Begin)>::bind());
		e.register_function("void End()", ImBinder<overload_of<void()>(&ImGui::End)>::bind());
		e.register_function("bool BeginChild(const string& str_id, const ImVec2& size, int child_flags = 0, int window_flags = 0)", ImBinder<overload_of<bool(const char *, const ImVec2 &, ImGuiChildFlags, ImGuiWindowFlags)>(&ImGui::BeginChild)>::bind());
		e.register_function("bool BeginChild(uint id, const ImVec2& size, int child_flags = 0, int window_flags = 0)", ImBinder<overload_of<bool(ImGuiID, const ImVec2 &, ImGuiChildFlags, ImGuiWindowFlags)>(&ImGui::BeginChild)>::bind());
		e.register_function("void EndChild()", ImBinder<overload_of<void()>(&ImGui::EndChild)>::bind());
		e.register_function("bool IsWindowAppearing()", ImBinder<overload_of<bool()>(&ImGui::IsWindowAppearing)>::bind());
		e.register_function("bool IsWindowCollapsed()", ImBinder<overload_of<bool()>(&ImGui::IsWindowCollapsed)>::bind());
		e.register_function("bool IsWindowFocused(int flags = 0)", ImBinder<overload_of<bool(ImGuiFocusedFlags)>(&ImGui::IsWindowFocused)>::bind());
		e.register_function("bool IsWindowHovered(int flags = 0)", ImBinder<overload_of<bool(ImGuiHoveredFlags)>(&ImGui::IsWindowHovered)>::bind());
		// e.register_function("ImDrawList@ GetWindowDrawList()", ImBinder<overload_of<ImDrawList *>(&ImGui::GetWindowDrawList)>::bind());
		e.register_function("float GetWindowDpiScale()", ImBinder<overload_of<float()>(&ImGui::GetWindowDpiScale)>::bind());
		e.register_function("ImVec2 GetWindowPos()", ImBinder<overload_of<ImVec2()>(&ImGui::GetWindowPos)>::bind());
		e.register_function("ImVec2 GetWindowSize()", ImBinder<overload_of<ImVec2()>(&ImGui::GetWindowSize)>::bind());
		e.register_function("float GetWindowWidth()", ImBinder<overload_of<float()>(&ImGui::GetWindowWidth)>::bind());
		e.register_function("float GetWindowHeight()", ImBinder<overload_of<float()>(&ImGui::GetWindowHeight)>::bind());
		// e.register_function("ImGuiViewport@ GetWindowViewport()", ImBinder<overload_of<ImGuiViewport *>(&ImGui::GetWindowViewport)>::bind());
		e.register_function("void SetNextWindowPos(const ImVec2& pos, int cond = 0, const ImVec2& pivot = ImVec2())", ImBinder<overload_of<void(const ImVec2 &, ImGuiCond, const ImVec2 &)>(&ImGui::SetNextWindowPos)>::bind());
		e.register_function("void SetNextWindowSize(const ImVec2 & size, int cond = 0)", ImBinder<overload_of<void(const ImVec2 &, ImGuiCond)>(&ImGui::SetNextWindowSize)>::bind());
		// e.register_function("void SetNextWindowSizeConstraints(const ImVec2 & size_min, const ImVec2 & size_max)", ImBinder<overload_of<void, const ImVec2 &, const ImVec2 &, ImGuiSizeCallback, void *>(&ImGui::SetNextWindowSizeConstraints)>::bind());
		e.register_function("void SetNextWindowContentSize(const ImVec2 & size)", ImBinder<overload_of<void(const ImVec2 &)>(&ImGui::SetNextWindowContentSize)>::bind());
		e.register_function("void SetNextWindowCollapsed(bool collapsed, int cond = 0)", ImBinder<overload_of<void(bool, ImGuiCond)>(&ImGui::SetNextWindowCollapsed)>::bind());
		e.register_function("void SetNextWindowFocus()", ImBinder<overload_of<void()>(&ImGui::SetNextWindowFocus)>::bind());
		e.register_function("void SetNextWindowScroll(const ImVec2 & scroll)", ImBinder<overload_of<void(const ImVec2 &)>(&ImGui::SetNextWindowScroll)>::bind());
		e.register_function("void SetNextWindowBgAlpha(float alpha)", ImBinder<overload_of<void(float)>(&ImGui::SetNextWindowBgAlpha)>::bind());
		e.register_function("void SetNextWindowViewport(uint viewport_id)", ImBinder<overload_of<void(ImGuiID)>(&ImGui::SetNextWindowViewport)>::bind());
		e.register_function("void SetWindowPos(const ImVec2 & pos, int cond = 0)", ImBinder<overload_of<void(const ImVec2 &, ImGuiCond)>(&ImGui::SetWindowPos)>::bind());
		e.register_function("void SetWindowSize(const ImVec2 & size, int cond = 0)", ImBinder<overload_of<void(const ImVec2 &, ImGuiCond)>(&ImGui::SetWindowSize)>::bind());
		e.register_function("void SetWindowCollapsed(bool collapsed, int cond = 0)", ImBinder<overload_of<void(bool, ImGuiCond)>(&ImGui::SetWindowCollapsed)>::bind());
		e.register_function("void SetWindowFocus()", ImBinder<overload_of<void()>(&ImGui::SetWindowFocus)>::bind());
		e.register_function("void SetWindowFontScale(float scale)", ImBinder<overload_of<void(float)>(&ImGui::SetWindowFontScale)>::bind());
		e.register_function("void SetWindowPos(const string& name, const ImVec2 & pos, int cond = 0)", ImBinder<overload_of<void(const char *, const ImVec2 &, ImGuiCond)>(&ImGui::SetWindowPos)>::bind());
		e.register_function("void SetWindowSize(const string& name, const ImVec2 & size, int cond = 0)", ImBinder<overload_of<void(const char *, const ImVec2 &, ImGuiCond)>(&ImGui::SetWindowSize)>::bind());
		e.register_function("void SetWindowCollapsed(const string& name, bool collapsed, int cond = 0)", ImBinder<overload_of<void(const char *, bool, ImGuiCond)>(&ImGui::SetWindowCollapsed)>::bind());
		e.register_function("void SetWindowFocus(const string& name)", ImBinder<overload_of<void(const char *)>(&ImGui::SetWindowFocus)>::bind());
		e.register_function("ImVec2 GetContentRegionAvail()", ImBinder<overload_of<ImVec2()>(&ImGui::GetContentRegionAvail)>::bind());
		e.register_function("ImVec2 GetContentRegionMax()", ImBinder<overload_of<ImVec2()>(&ImGui::GetContentRegionMax)>::bind());
		e.register_function("ImVec2 GetWindowContentRegionMin()", ImBinder<overload_of<ImVec2()>(&ImGui::GetWindowContentRegionMin)>::bind());
		e.register_function("ImVec2 GetWindowContentRegionMax()", ImBinder<overload_of<ImVec2()>(&ImGui::GetWindowContentRegionMax)>::bind());
		e.register_function("float GetScrollX()", ImBinder<overload_of<float()>(&ImGui::GetScrollX)>::bind());
		e.register_function("float GetScrollY()", ImBinder<overload_of<float()>(&ImGui::GetScrollY)>::bind());
		e.register_function("void SetScrollX(float scroll_x)", ImBinder<overload_of<void(float)>(&ImGui::SetScrollX)>::bind());
		e.register_function("void SetScrollY(float scroll_y)", ImBinder<overload_of<void(float)>(&ImGui::SetScrollY)>::bind());
		e.register_function("float GetScrollMaxX()", ImBinder<overload_of<float()>(&ImGui::GetScrollMaxX)>::bind());
		e.register_function("float GetScrollMaxY()", ImBinder<overload_of<float()>(&ImGui::GetScrollMaxY)>::bind());
		e.register_function("void SetScrollHereX(float center_x_ratio = 0.5f)", ImBinder<overload_of<void(float)>(&ImGui::SetScrollHereX)>::bind());
		e.register_function("void SetScrollHereY(float center_y_ratio = 0.5f)", ImBinder<overload_of<void(float)>(&ImGui::SetScrollHereY)>::bind());
		e.register_function("void SetScrollFromPosX(float local_x, float center_x_ratio = 0.5f)", ImBinder<overload_of<void(float, float)>(&ImGui::SetScrollFromPosX)>::bind());
		e.register_function("void SetScrollFromPosY(float local_y, float center_y_ratio = 0.5f)", ImBinder<overload_of<void(float, float)>(&ImGui::SetScrollFromPosY)>::bind());
		//e.register_function("void PushFont(Ptr<ImFont> font)", ImBinder<overload_of<void, ImFont *>(&ImGui::PushFont)>::bind());
		e.register_function("void PopFont()", ImBinder<overload_of<void()>(&ImGui::PopFont)>::bind());
		e.register_function("void PushStyleColor(int idx, uint col)", ImBinder<overload_of<void(ImGuiCol, ImU32)>(&ImGui::PushStyleColor)>::bind());
		e.register_function("void PushStyleColor(int idx, const ImVec4 & col)", ImBinder<overload_of<void(ImGuiCol, const ImVec4 &)>(&ImGui::PushStyleColor)>::bind());
		e.register_function("void PopStyleColor(int count = 1)", ImBinder<overload_of<void(int)>(&ImGui::PopStyleColor)>::bind());
		e.register_function("void PushStyleVar(int idx, float val)", ImBinder<overload_of<void(ImGuiStyleVar, float)>(&ImGui::PushStyleVar)>::bind());
		e.register_function("void PushStyleVar(int idx, const ImVec2 & val)", ImBinder<overload_of<void(ImGuiStyleVar, const ImVec2 &)>(&ImGui::PushStyleVar)>::bind());
		e.register_function("void PopStyleVar(int count = 1)", ImBinder<overload_of<void(int)>(&ImGui::PopStyleVar)>::bind());
		e.register_function("void PushTabStop(bool tab_stop)", ImBinder<overload_of<void(bool)>(&ImGui::PushTabStop)>::bind());
		e.register_function("void PopTabStop()", ImBinder<overload_of<void()>(&ImGui::PopTabStop)>::bind());
		e.register_function("void PushButtonRepeat(bool repeat)", ImBinder<overload_of<void(bool)>(&ImGui::PushButtonRepeat)>::bind());
		e.register_function("void PopButtonRepeat()", ImBinder<overload_of<void()>(&ImGui::PopButtonRepeat)>::bind());
		e.register_function("void PushItemWidth(float item_width)", ImBinder<overload_of<void(float)>(&ImGui::PushItemWidth)>::bind());
		e.register_function("void PopItemWidth()", ImBinder<overload_of<void()>(&ImGui::PopItemWidth)>::bind());
		e.register_function("void SetNextItemWidth(float item_width)", ImBinder<overload_of<void(float)>(&ImGui::SetNextItemWidth)>::bind());
		e.register_function("float CalcItemWidth()", ImBinder<overload_of<float()>(&ImGui::CalcItemWidth)>::bind());
		e.register_function("void PushTextWrapPos(float wrap_local_pos_x = 0.0f)", ImBinder<overload_of<void(float)>(&ImGui::PushTextWrapPos)>::bind());
		e.register_function("void PopTextWrapPos()", ImBinder<overload_of<void()>(&ImGui::PopTextWrapPos)>::bind());
		// e.register_function("Ptr<ImFont> GetFont()", ImBinder<overload_of<ImFont *()>(&ImGui::GetFont)>::bind());
		e.register_function("float GetFontSize()", ImBinder<overload_of<float()>(&ImGui::GetFontSize)>::bind());
		e.register_function("ImVec2 GetFontTexUvWhitePixel()", ImBinder<overload_of<ImVec2()>(&ImGui::GetFontTexUvWhitePixel)>::bind());
		e.register_function("uint GetColorU32(int idx, float alpha_mul = 1.0f)", ImBinder<overload_of<ImU32(ImGuiCol, float)>(&ImGui::GetColorU32)>::bind());
		e.register_function("uint GetColorU32(const ImVec4 & col)", ImBinder<overload_of<ImU32(const ImVec4 &)>(&ImGui::GetColorU32)>::bind());
		e.register_function("uint GetColorU32(uint col, float alpha_mul = 1.0f)", ImBinder<overload_of<ImU32(ImU32, float)>(&ImGui::GetColorU32)>::bind());
		e.register_function("const ImVec4 & GetStyleColorVec4(int idx)", ImBinder<overload_of<const ImVec4 &(ImGuiCol)>(&ImGui::GetStyleColorVec4)>::bind());
		e.register_function("ImVec2 GetCursorScreenPos()", ImBinder<overload_of<ImVec2()>(&ImGui::GetCursorScreenPos)>::bind());
		e.register_function("void SetCursorScreenPos(const ImVec2 & pos)", ImBinder<overload_of<void(const ImVec2 &)>(&ImGui::SetCursorScreenPos)>::bind());
		e.register_function("ImVec2 GetCursorPos()", ImBinder<overload_of<ImVec2()>(&ImGui::GetCursorPos)>::bind());
		e.register_function("float GetCursorPosX()", ImBinder<overload_of<float()>(&ImGui::GetCursorPosX)>::bind());
		e.register_function("float GetCursorPosY()", ImBinder<overload_of<float()>(&ImGui::GetCursorPosY)>::bind());
		e.register_function("void SetCursorPos(const ImVec2 & local_pos)", ImBinder<overload_of<void(const ImVec2 &)>(&ImGui::SetCursorPos)>::bind());
		e.register_function("void SetCursorPosX(float local_x)", ImBinder<overload_of<void(float)>(&ImGui::SetCursorPosX)>::bind());
		e.register_function("void SetCursorPosY(float local_y)", ImBinder<overload_of<void(float)>(&ImGui::SetCursorPosY)>::bind());
		e.register_function("ImVec2 GetCursorStartPos()", ImBinder<overload_of<ImVec2()>(&ImGui::GetCursorStartPos)>::bind());
		e.register_function("void Separator()", ImBinder<overload_of<void()>(&ImGui::Separator)>::bind());
		e.register_function("void SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f)", ImBinder<overload_of<void(float, float)>(&ImGui::SameLine)>::bind());
		e.register_function("void NewLine()", ImBinder<overload_of<void()>(&ImGui::NewLine)>::bind());
		e.register_function("void Spacing()", ImBinder<overload_of<void()>(&ImGui::Spacing)>::bind());
		e.register_function("void Dummy(const ImVec2 & size)", ImBinder<overload_of<void(const ImVec2 &)>(&ImGui::Dummy)>::bind());
		e.register_function("void Indent(float indent_w = 0.0f)", ImBinder<overload_of<void(float)>(&ImGui::Indent)>::bind());
		e.register_function("void Unindent(float indent_w = 0.0f)", ImBinder<overload_of<void(float)>(&ImGui::Unindent)>::bind());
		e.register_function("void BeginGroup()", ImBinder<overload_of<void()>(&ImGui::BeginGroup)>::bind());
		e.register_function("void EndGroup()", ImBinder<overload_of<void()>(&ImGui::EndGroup)>::bind());
		e.register_function("void AlignTextToFramePadding()", ImBinder<overload_of<void()>(&ImGui::AlignTextToFramePadding)>::bind());
		e.register_function("float GetTextLineHeight()", ImBinder<overload_of<float()>(&ImGui::GetTextLineHeight)>::bind());
		e.register_function("float GetTextLineHeightWithSpacing()", ImBinder<overload_of<float()>(&ImGui::GetTextLineHeightWithSpacing)>::bind());
		e.register_function("float GetFrameHeight()", ImBinder<overload_of<float()>(&ImGui::GetFrameHeight)>::bind());
		e.register_function("float GetFrameHeightWithSpacing()", ImBinder<overload_of<float()>(&ImGui::GetFrameHeightWithSpacing)>::bind());
		e.register_function("void PushID(const string& str_id)", ImBinder<overload_of<void(const char *)>(&ImGui::PushID)>::bind());
		e.register_function("void PushID(int int_id)", ImBinder<overload_of<void(int)>(&ImGui::PushID)>::bind());
		e.register_function("void PushID(Ptr<void> ptr_id)", ImBinder<overload_of<void(const void *)>(&ImGui::PushID)>::bind());
		e.register_function("void PopID()", ImBinder<overload_of<void()>(&ImGui::PopID)>::bind());;
		e.register_function("uint GetID(const string& str_id)", ImBinder<overload_of<ImGuiID(const char *)>(&ImGui::GetID)>::bind());
		e.register_function("uint GetID(Ptr<void> ptr_id)", ImBinder<overload_of<ImGuiID(const void *)>(&ImGui::GetID)>::bind());
		e.register_function("void Text(const string& text)", Custom::Text);
		e.register_function("void TextColored(const ImVec4 & col, const string& text)", Custom::TextColored);
		e.register_function("void TextDisabled(const string& text)", Custom::TextDisabled);
		e.register_function("void TextWrapped(const string& text)", Custom::TextWrapped);
		e.register_function("void LabelText(const string& label, const string& text)", Custom::LabelText);
		e.register_function("void BulletText(const string& text)", Custom::BulletText);
		e.register_function("void SeparatorText(const string& label)", ImBinder<overload_of<void(const char *)>(&ImGui::SeparatorText)>::bind());
		e.register_function("bool Button(const string& label, const ImVec2& size = ImVec2())", ImBinder<overload_of<bool(const char *, const ImVec2 &)>(&ImGui::Button)>::bind());
		e.register_function("bool SmallButton(const string& label)", ImBinder<overload_of<bool(const char *)>(&ImGui::SmallButton)>::bind());
		e.register_function("bool InvisibleButton(const string& str_id, const ImVec2 & size, int flags = 0)", ImBinder<overload_of<bool(const char *, const ImVec2 &, ImGuiButtonFlags)>(&ImGui::InvisibleButton)>::bind());
		e.register_function("bool ArrowButton(const string& str_id, int dir)", ImBinder<overload_of<bool(const char *, ImGuiDir)>(&ImGui::ArrowButton)>::bind());
		e.register_function("bool Checkbox(const string& label, bool& v)", Custom::Checkbox);
		e.register_function("bool CheckboxFlags(const string& label, uint64& flags, uint64 flags_value)", Custom::CheckboxFlags);
		e.register_function("bool RadioButton(const string& label, bool active)", ImBinder<overload_of<bool(const char *, bool)>(&ImGui::RadioButton)>::bind());
		e.register_function("bool RadioButton(const string& label, int& v, int v_button)", Custom::RadioButton);
		e.register_function("void ProgressBar(float fraction, const ImVec2 & size_arg, const string& overlay = nullptr)", ImBinder<overload_of<void(float, const ImVec2 &, const char *)>(&ImGui::ProgressBar)>::bind());
		e.register_function("void Bullet()", ImBinder<overload_of<void()>(&ImGui::Bullet)>::bind());
		// e.register_function("void Image(ImTextureID user_texture_id, const ImVec2 & size, const ImVec2 & uv0, const ImVec2 & uv1, const ImVec4 & tint_col, const ImVec4 & border_col)", ImBinder<overload_of<void, ImTextureID, const ImVec2 &, const ImVec2 &, const ImVec2 &, const ImVec4 &, const ImVec4 &>(&ImGui::Image)>::bind());
		// e.register_function("bool ImageButton(const string& str_id, ImTextureID user_texture_id, const ImVec2 & image_size, const ImVec2 & uv0, const ImVec2 & uv1, const ImVec4 & bg_col, const ImVec4 & tint_col)", ImBinder<overload_of<bool, const char *, ImTextureID, const ImVec2 &, const ImVec2 &, const ImVec2 &, const ImVec4 &, const ImVec4 &>(&ImGui::ImageButton)>::bind());
		e.register_function("bool BeginCombo(const string& label, const string& preview_value, int flags = 0)", ImBinder<overload_of<bool(const char *, const char *, ImGuiComboFlags)>(&ImGui::BeginCombo)>::bind());
		e.register_function("void EndCombo()", ImBinder<overload_of<void()>(&ImGui::EndCombo)>::bind());
		e.register_function("bool Combo(const string &in label, int &inout current_item, const Engine::Vector<string> &in items, int popup_max_height_in_items = -1)", Custom::Combo);
		e.register_function("bool Combo(const string& label, int& current_item, const string& items_separated_by_zeros, int popup_max_height_in_items = -1)", Custom::Combo2);
		e.register_function("bool DragFloat(const string& label, float& v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const string& format = \"%.3f\", int flags = 0)", ImBinder<ImGui::DragFloat, ImArg<1, float[1]>>::bind());
		e.register_function("bool DragFloat2(const string& label, ImVec2& v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const string& format = \"%.3f\", int flags = 0)", ImBinder<ImGui::DragFloat2, ImArg<1, float[2]>>::bind());
		e.register_function("bool DragFloat3(const string& label, ImVec3& v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const string& format = \"%.3f\", int flags = 0)", ImBinder<ImGui::DragFloat3, ImArg<1, float[3]>>::bind());
		e.register_function("bool DragFloat4(const string& label, ImVec4& v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const string& format = \"%.3f\", int flags = 0)", ImBinder<ImGui::DragFloat4, ImArg<1, float[4]>>::bind());
		e.register_function("bool DragFloatRange2(const string& label, float& v_current_min, float& v_current_max, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const string& format = \"%.3f\", const string& format_max = \"\", int flags = 0)", ImBinder<&ImGui::DragFloatRange2, ImArg<1, float[1]>, ImArg<2, float[1]>>::bind());
		e.register_function("bool DragInt(const string& label, int& v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const string& format = \"%d\", int flags = 0)", ImBinder<&ImGui::DragInt, ImArg<1, int[1]>>::bind());
		e.register_function("bool DragInt2(const string& label, ImIVec2& v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const string& format = \"%d\", int flags = 0)", ImBinder<&ImGui::DragInt2, ImArg<1, int[2]>>::bind());
		e.register_function("bool DragInt3(const string& label, ImIVec3& v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const string& format = \"%d\", int flags = 0)", ImBinder<&ImGui::DragInt3, ImArg<1, int[3]>>::bind());
		e.register_function("bool DragInt4(const string& label, ImIVec4& v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const string& format = \"%d\", int flags = 0)", ImBinder<&ImGui::DragInt4, ImArg<1, int[4]>>::bind());
		e.register_function("bool DragIntRange2(const string& label, int& v_current_min, int& v_current_max, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const string& format = \"%d\", const string& format_max = \"\", int flags = 0)", ImBinder<&ImGui::DragIntRange2, ImArg<1, int[1]>, ImArg<2, int[1]>>::bind());
		e.register_function("bool DragScalar(const string& label, int data_type, Ptr<void> p_data, float v_speed = 1.0f, Ptr<void> p_min = nullptr, Ptr<void> p_max = nullptr, const string& format = \"\", int flags = 0)", ImBinder<&ImGui::DragScalar, ImArg<6, NullableString>>::bind());
		e.register_function("bool DragScalarN(const string& label, int data_type, Ptr<void> p_data, int components, float v_speed = 1.0f, Ptr<void> p_min = nullptr, Ptr<void> p_max = nullptr, const string& format = \"\", int flags = 0)", ImBinder<&ImGui::DragScalarN, ImArg<7, NullableString>>::bind());
		e.register_function("bool SliderFloat(const string& label, float& v, float v_min, float v_max, const string& format = \"%.3f\", int flags = 0)", ImBinder<&ImGui::SliderFloat, ImArg<1, float[1]>>::bind());
		e.register_function("bool SliderFloat2(const string& label, ImVec2& v, float v_min, float v_max, const string& format = \"%.3f\", int flags = 0)", ImBinder<&ImGui::SliderFloat2, ImArg<1, float[2]>>::bind());
		e.register_function("bool SliderFloat3(const string& label, ImVec3& v, float v_min, float v_max, const string& format = \"%.3f\", int flags = 0)", ImBinder<&ImGui::SliderFloat3, ImArg<1, float[3]>>::bind());
		e.register_function("bool SliderFloat4(const string& label, ImVec4& v, float v_min, float v_max, const string& format = \"%.3f\", int flags = 0)", ImBinder<&ImGui::SliderFloat4, ImArg<1, float[4]>>::bind());
		e.register_function("bool SliderAngle(const string& label, float& v_rad, float v_degrees_min, float v_degrees_max, const string& format = \"%.0f deg\", int flags = 0)", ImBinder<&ImGui::SliderAngle>::bind());
		e.register_function("bool SliderInt(const string& label, int& v, int v_min, int v_max, const string& format = \"%d\", int flags = 0)", ImBinder<&ImGui::SliderInt, ImArg<1, int[1]>>::bind());
		e.register_function("bool SliderInt2(const string& label, ImIVec2& v, int v_min, int v_max, const string& format = \"%d\", int flags = 0)", ImBinder<&ImGui::SliderInt2, ImArg<1, int[2]>>::bind());
		e.register_function("bool SliderInt3(const string& label, ImIVec3& v, int v_min, int v_max, const string& format = \"%d\", int flags = 0)", ImBinder<&ImGui::SliderInt3, ImArg<1, int[3]>>::bind());
		e.register_function("bool SliderInt4(const string& label, ImIVec4& v, int v_min, int v_max, const string& format = \"%d\", int flags = 0)", ImBinder<&ImGui::SliderInt4, ImArg<1, int[4]>>::bind());
		e.register_function("bool SliderScalar(const string& label, int data_type, Ptr<void> p_data, Ptr<void> p_min, Ptr<void> p_max, const string& format = \"\", int flags = 0)", ImBinder<&ImGui::SliderScalar, ImArg<5 ,NullableString>>::bind());
		e.register_function("bool SliderScalarN(const string& label, int data_type, Ptr<void> p_data, int components, Ptr<void> p_min, Ptr<void> p_max, const string& format = \"\", int flags = 0)", ImBinder<&ImGui::SliderScalarN, ImArg<6 ,NullableString>>::bind());
		e.register_function("bool VSliderFloat(const string& label, const ImVec2& size, float& v, float v_min, float v_max, const string& format = \"%.3f\", int flags = 0)", ImBinder<&ImGui::VSliderFloat, ImArg<2, float[1]>, ImArg<5 ,NullableString>>::bind());
		e.register_function("bool VSliderInt(const string& label, const ImVec2& size, int& v, int v_min, int v_max, const string& format = \"%d\", int flags = 0)", ImBinder<&ImGui::VSliderInt, ImArg<2, int[1]>, ImArg<5 ,NullableString>>::bind());
		e.register_function("bool VSliderScalar(const string& label, const ImVec2& size, int data_type, Ptr<void> p_data, Ptr<void> p_min, Ptr<void> p_max, const string& format = \"\", int flags = 0)", ImBinder<&ImGui::VSliderScalar, ImArg<6, NullableString>>::bind());
		e.register_function("bool InputText(const string& label, string& buf, int flags = 0)", Custom::InputText);
		e.register_function("bool InputTextMultiline(const string& label, string& buf, const ImVec2& size = ImVec2(), int flags = 0)", Custom::InputTextMultiline);
		e.register_function("bool InputTextWithHint(const string& label, const string& hint, string& buf, int flags = 0)", Custom::InputTextWithHint);
		e.register_function("bool InputFloat(const string& label, float& v, float step = 0.0f, float step_fast = 0.0f, const string& format = \"%.3f\", int flags = 0)", ImBinder<&ImGui::InputFloat, ImArg<1, float[1]>>::bind());
		e.register_function("bool InputFloat2(const string& label, ImVec2& v, const string& format = \"%.3f\", int flags = 0)", ImBinder<&ImGui::InputFloat2, ImArg<1, float[2]>>::bind());
		e.register_function("bool InputFloat3(const string& label, ImVec3& v, const string& format = \"%.3f\", int flags = 0)", ImBinder<&ImGui::InputFloat3, ImArg<1, float[3]>>::bind());
		e.register_function("bool InputFloat4(const string& label, ImVec4& v, const string& format = \"%.3f\", int flags = 0)", ImBinder<&ImGui::InputFloat4, ImArg<1, float[4]>>::bind());
		e.register_function("bool InputInt(const string& label, int& v, int step = 1, int step_fast = 100, int flags = 0)", ImBinder<&ImGui::InputInt, ImArg<1, int[1]>>::bind());
		e.register_function("bool InputInt2(const string& label, ImIVec2& v, int flags = 0)", ImBinder<&ImGui::InputInt2, ImArg<1, int[2]>>::bind());
		e.register_function("bool InputInt3(const string& label, ImIVec3& v, int flags = 0)", ImBinder<&ImGui::InputInt3, ImArg<1, int[3]>>::bind());
		e.register_function("bool InputInt4(const string& label, ImIVec4& v, int flags = 0)", ImBinder<&ImGui::InputInt4, ImArg<1, int[4]>>::bind());
		e.register_function("bool InputDouble(const string& label, double& v, double step = 0.0, double step_fast = 0.0, const string& format = \"%.6f\", int flags = 0)", ImBinder<&ImGui::InputDouble, ImArg<1, double[1]>>::bind());
		e.register_function("bool InputScalar(const string& label, int data_type, Ptr<void> p_data, Ptr<void> p_step = nullptr, Ptr<void> p_step_fast = nullptr, const string& format = \"\", int flags = 0)", ImBinder<&ImGui::InputScalar, ImArg<5, NullableString>>::bind());
		e.register_function("bool InputScalarN(const string& label, int data_type, Ptr<void> p_data, int components, Ptr<void> p_step = nullptr, Ptr<void> p_step_fast = nullptr, const string& format = \"\", int flags = 0)", ImBinder<&ImGui::InputScalarN, ImArg<6, NullableString>>::bind());
		e.register_function("bool ColorEdit3(const string& label, ImVec3& col, int flags = 0)", ImBinder<&ImGui::ColorEdit3, ImArg<1, float[3]>>::bind());
		e.register_function("bool ColorEdit4(const string& label, ImVec4& col, int flags = 0)", ImBinder<&ImGui::ColorEdit4, ImArg<1, float[4]>>::bind());
		e.register_function("bool ColorPicker3(const string& label, ImVec3& col, int flags = 0)", ImBinder<&ImGui::ColorPicker3, ImArg<1, float[3]>>::bind());
		e.register_function("bool ColorPicker4(const string& label, ImVec4& col, int flags = 0, Ptr<ImVec4> ref_col = nullptr)", ImBinder<&ImGui::ColorPicker4, ImArg<1, float[4]>>::bind());
		e.register_function("bool ColorButton(const string& desc_id, const ImVec4& col, int flags = 0, const ImVec2& size = ImVec2())", ImBinder<&ImGui::ColorButton>::bind());
		e.register_function("void SetColorEditOptions(int flags)", ImBinder<&ImGui::SetColorEditOptions>::bind());
		e.register_function("bool TreeNode(const string& label)", ImBinder<overload_of<bool(const char *)>(&ImGui::TreeNode)>::bind());
		e.register_function("bool TreeNode(Ptr<void> ptr_id, const string& fmt)", Custom::TreeNode);
		e.register_function("bool TreeNodeEx(const string& label, int flags = 0)", ImBinder<overload_of<bool(const char *, ImGuiTreeNodeFlags)>(&ImGui::TreeNodeEx)>::bind());
		e.register_function("bool TreeNodeEx(const string& str_id, const string& text, int flags)", Custom::TreeNodeEx);
		e.register_function("bool TreeNodeEx(Ptr<void> ptr_id, const string& text, int flags = 0)", Custom::TreeNodeEx2);
		e.register_function("void TreePush(const string& str_id)", ImBinder<overload_of<void(const char *)>(&ImGui::TreePush)>::bind());
		e.register_function("void TreePush(Ptr<void> ptr_id)", ImBinder<overload_of<void(const void *)>(&ImGui::TreePush)>::bind());
		e.register_function("void TreePop()", ImBinder<&ImGui::TreePop>::bind());
		e.register_function("float GetTreeNodeToLabelSpacing()", ImBinder<&ImGui::GetTreeNodeToLabelSpacing>::bind());
		e.register_function("bool CollapsingHeader(const string& label, int flags = 0)", ImBinder<overload_of<bool(const char *, ImGuiTreeNodeFlags)>(&ImGui::CollapsingHeader)>::bind());
		e.register_function("bool CollapsingHeader(const string& label, bool& p_visible, int flags = 0)", ImBinder<overload_of<bool(const char *, bool *, ImGuiTreeNodeFlags)>(&ImGui::CollapsingHeader), ImArg<1, bool[1]>>::bind());
		e.register_function("void SetNextItemOpen(bool is_open, int cond = 0)", ImBinder<&ImGui::SetNextItemOpen>::bind());
		e.register_function("bool Selectable(const string& label, bool selected = false, int flags = 0, const ImVec2& size = ImVec2())", ImBinder<overload_of<bool(const char *, bool, ImGuiSelectableFlags, const ImVec2 &)>(&ImGui::Selectable)>::bind());
		e.register_function("bool SelectableToggle(const string& label, bool& p_selected, int flags = 0, const ImVec2& size = ImVec2())", ImBinder<overload_of<bool(const char *, bool *, ImGuiSelectableFlags, const ImVec2&)>(&ImGui::Selectable), ImArg<1, bool[1]>>::bind());
		e.register_function("bool BeginListBox(const string& label, const ImVec2& size = ImVec2())", ImBinder<&ImGui::BeginListBox>::bind());
		e.register_function("void EndListBox()", ImBinder<&ImGui::EndListBox>::bind());

		// e.register_function("void PlotLines(const string& label, Ptr<float> values, int values_count, int values_offset = 0, const string& overlay_text = nullptr, float scale_min, float scale_max, ImVec2 graph_size, int stride)", ImBinder<overload_of<void, const char *, const float *, int, int, const char *, float, float, ImVec2, int>(&ImGui::PlotLines)>::bind());
		// e.register_function("void PlotLines(const string& label, Ptr<int)> values_getter, Ptr<void> data, int values_count, int values_offset = 0, const string& overlay_text = nullptr, float scale_min, float scale_max, ImVec2 graph_size)", ImBinder<overload_of<void, const char *, float (*)(void *, int), void *, int, int, const char *, float, float, ImVec2>(&ImGui::PlotLines)>::bind());
		// e.register_function("void PlotHistogram(const string& label, Ptr<float> values, int values_count, int values_offset = 0, const string& overlay_text = nullptr, float scale_min, float scale_max, ImVec2 graph_size, int stride)", ImBinder<overload_of<void, const char *, const float *, int, int, const char *, float, float, ImVec2, int>(&ImGui::PlotHistogram)>::bind());
		// e.register_function("void PlotHistogram(const string& label, Ptr<int)> values_getter, Ptr<void> data, int values_count, int values_offset = 0, const string& overlay_text = nullptr, float scale_min, float scale_max, ImVec2 graph_size)", ImBinder<overload_of<void, const char *, float (*)(void *, int), void *, int, int, const char *, float, float, ImVec2>(&ImGui::PlotHistogram)>::bind());
		// e.register_function("void Value(const string& prefix, bool b)", ImBinder<overload_of<void, const char *, bool>(&ImGui::Value)>::bind());
		// e.register_function("void Value(const string& prefix, int v)", ImBinder<overload_of<void, const char *, int>(&ImGui::Value)>::bind());
		// e.register_function("void Value(const string& prefix, unsigned int v)", ImBinder<overload_of<void, const char *, unsigned int>(&ImGui::Value)>::bind());
		// e.register_function("void Value(const string& prefix, float v, const string& float_format = nullptr)", ImBinder<overload_of<void, const char *, float, const char *>(&ImGui::Value)>::bind());
		e.register_function("bool BeginMenuBar()", ImBinder<&ImGui::BeginMenuBar>::bind());
		e.register_function("void EndMenuBar()", ImBinder<&ImGui::EndMenuBar>::bind());
		e.register_function("bool BeginMainMenuBar()", ImBinder<&ImGui::BeginMainMenuBar>::bind());
		e.register_function("void EndMainMenuBar()", ImBinder<&ImGui::EndMainMenuBar>::bind());
		e.register_function("bool BeginMenu(const string& label, bool enabled = true)", ImBinder<&ImGui::BeginMenu>::bind());
		e.register_function("void EndMenu()", ImBinder<&ImGui::EndMenu>::bind());
		// e.register_function("bool MenuItem(const string& label, const string& shortcut = nullptr, bool selected = false, bool enabled = true)", ImBinder<overload_of<bool, const char *, const char *, bool, bool>(&ImGui::MenuItem)>::bind());
		// e.register_function("bool MenuItem(const string& label, const string& shortcut, Ptr<bool> p_selected, bool enabled = true)", ImBinder<overload_of<bool, const char *, const char *, bool *, bool>(&ImGui::MenuItem)>::bind());
		e.register_function("bool BeginTooltip()", ImBinder<&ImGui::BeginTooltip>::bind());
		e.register_function("void EndTooltip()", ImBinder<&ImGui::EndTooltip>::bind());
		// e.register_function("void SetTooltip(const string& fmt)", ImBinder<&ImGui::SetTooltip>::bind());
		// e.register_function("void SetTooltipV(const string& fmt, int args)", ImBinder<&ImGui::SetTooltipV>::bind());
		e.register_function("bool BeginItemTooltip()", ImBinder<&ImGui::BeginItemTooltip>::bind());
		// e.register_function("void SetItemTooltip(const string& fmt)", ImBinder<&ImGui::SetItemTooltip>::bind());

		e.register_function("void SetItemTooltipV(const string& fmt, int args)", ImBinder<&ImGui::SetItemTooltipV>::bind());
		// e.register_function("bool BeginPopup(const string& str_id, int flags = 0)", ImBinder<&ImGui::BeginPopup>::bind());
		// e.register_function("bool BeginPopupModal(const string& name, Ptr<bool> p_open = nullptr, int flags = 0)", ImBinder<&ImGui::BeginPopupModal>::bind());
		// e.register_function("void EndPopup()", ImBinder<&ImGui::EndPopup>::bind());
		// e.register_function("void OpenPopup(const string& str_id, int popup_flags = 0)", ImBinder<overload_of<void, const char *, ImGuiPopupFlags>(&ImGui::OpenPopup)>::bind());
		// e.register_function("void OpenPopup(unsigned int id, int popup_flags = 0)", ImBinder<overload_of<void, ImGuiID, ImGuiPopupFlags>(&ImGui::OpenPopup)>::bind());
		// e.register_function("void OpenPopupOnItemClick(const string& str_id = nullptr, int popup_flags = 1)", ImBinder<&ImGui::OpenPopupOnItemClick>::bind());
		// e.register_function("void CloseCurrentPopup()", ImBinder<&ImGui::CloseCurrentPopup>::bind());
		// e.register_function("bool BeginPopupContextItem(const string& str_id = nullptr, int popup_flags = 1)", ImBinder<&ImGui::BeginPopupContextItem>::bind());
		// e.register_function("bool BeginPopupContextWindow(const string& str_id = nullptr, int popup_flags = 1)", ImBinder<&ImGui::BeginPopupContextWindow>::bind());
		// e.register_function("bool BeginPopupContextVoid(const string& str_id = nullptr, int popup_flags = 1)", ImBinder<&ImGui::BeginPopupContextVoid>::bind());
		// e.register_function("bool IsPopupOpen(const string& str_id, int flags = 0)", ImBinder<&ImGui::IsPopupOpen>::bind());
		// e.register_function("bool BeginTable(const string& str_id, int column, int flags = 0, const ImVec2 & outer_size, float inner_width = 0.0f)", ImBinder<&ImGui::BeginTable>::bind());
		e.register_function("void EndTable()", ImBinder<&ImGui::EndTable>::bind());
		// e.register_function("void TableNextRow(int row_flags = 0, float min_row_height = 0.0f)", ImBinder<&ImGui::TableNextRow>::bind());
		e.register_function("bool TableNextColumn()", ImBinder<&ImGui::TableNextColumn>::bind());
		e.register_function("bool TableSetColumnIndex(int column_n)", ImBinder<&ImGui::TableSetColumnIndex>::bind());
		// e.register_function("void TableSetupColumn(const string& label, int flags = 0, float init_width_or_weight = 0.0f, unsigned int user_id = 0)", ImBinder<&ImGui::TableSetupColumn>::bind());
		// e.register_function("void TableSetupScrollFreeze(int cols, int rows)", ImBinder<&ImGui::TableSetupScrollFreeze>::bind());
		e.register_function("void TableHeader(const string& label)", ImBinder<&ImGui::TableHeader>::bind());
		e.register_function("void TableHeadersRow()", ImBinder<&ImGui::TableHeadersRow>::bind());
		e.register_function("void TableAngledHeadersRow()", ImBinder<&ImGui::TableAngledHeadersRow>::bind());
		// e.register_function("Ptr<ImGuiTableSortSpecs> TableGetSortSpecs()", ImBinder<&ImGui::TableGetSortSpecs>::bind());
		e.register_function("int TableGetColumnCount()", ImBinder<&ImGui::TableGetColumnCount>::bind());
		e.register_function("int TableGetColumnIndex()", ImBinder<&ImGui::TableGetColumnIndex>::bind());
		e.register_function("int TableGetRowIndex()", ImBinder<&ImGui::TableGetRowIndex>::bind());
		e.register_function("string TableGetColumnName(int column_n = -1)", ImBinder<overload_of<const char*(int)>(&ImGui::TableGetColumnName)>::bind());
		// e.register_function("int TableGetColumnFlags(int column_n)", ImBinder<&ImGui::TableGetColumnFlags>::bind());
		// e.register_function("void TableSetColumnEnabled(int column_n, bool v)", ImBinder<&ImGui::TableSetColumnEnabled>::bind());
		// e.register_function("void TableSetBgColor(int target, unsigned int color, int column_n)", ImBinder<&ImGui::TableSetBgColor>::bind());
		// e.register_function("void Columns(int count = 1, const string& id = nullptr, bool border = true)", ImBinder<&ImGui::Columns>::bind());
		// e.register_function("void NextColumn()", ImBinder<&ImGui::NextColumn>::bind());
		// e.register_function("int GetColumnIndex()", ImBinder<&ImGui::GetColumnIndex>::bind());
		// e.register_function("float GetColumnWidth(int column_index)", ImBinder<&ImGui::GetColumnWidth>::bind());
		// e.register_function("void SetColumnWidth(int column_index, float width)", ImBinder<&ImGui::SetColumnWidth>::bind());
		// e.register_function("float GetColumnOffset(int column_index)", ImBinder<&ImGui::GetColumnOffset>::bind());
		// e.register_function("void SetColumnOffset(int column_index, float offset_x)", ImBinder<&ImGui::SetColumnOffset>::bind());
		// e.register_function("int GetColumnsCount()", ImBinder<&ImGui::GetColumnsCount>::bind());
		// e.register_function("bool BeginTabBar(const string& str_id, int flags = 0)", ImBinder<&ImGui::BeginTabBar>::bind());
		// e.register_function("void EndTabBar()", ImBinder<&ImGui::EndTabBar>::bind());
		// e.register_function("bool BeginTabItem(const string& label, Ptr<bool> p_open = nullptr, int flags = 0)", ImBinder<&ImGui::BeginTabItem>::bind());
		// e.register_function("void EndTabItem()", ImBinder<&ImGui::EndTabItem>::bind());
		// e.register_function("bool TabItemButton(const string& label, int flags = 0)", ImBinder<&ImGui::TabItemButton>::bind());
		// e.register_function("void SetTabItemClosed(const string& tab_or_docked_window_label)", ImBinder<&ImGui::SetTabItemClosed>::bind());
		// e.register_function("unsigned int DockSpace(unsigned int id, const ImVec2 & size, int flags = 0, Ptr<ImGuiWindowClass> window_class = nullptr)", ImBinder<&ImGui::DockSpace>::bind());
		// e.register_function("unsigned int DockSpaceOverViewport(Ptr<ImGuiViewport> viewport = nullptr, int flags = 0, Ptr<ImGuiWindowClass> window_class = nullptr)", ImBinder<&ImGui::DockSpaceOverViewport>::bind());
		// e.register_function("void SetNextWindowDockID(unsigned int dock_id, int cond = 0)", ImBinder<&ImGui::SetNextWindowDockID>::bind());
		// e.register_function("void SetNextWindowClass(Ptr<ImGuiWindowClass> window_class)", ImBinder<&ImGui::SetNextWindowClass>::bind());
		// e.register_function("unsigned int GetWindowDockID()", ImBinder<&ImGui::GetWindowDockID>::bind());
		// e.register_function("bool IsWindowDocked()", ImBinder<&ImGui::IsWindowDocked>::bind());
		// e.register_function("void LogToTTY(int auto_open_depth)", ImBinder<&ImGui::LogToTTY>::bind());
		// e.register_function("void LogToFile(int auto_open_depth, const string& filename = nullptr)", ImBinder<&ImGui::LogToFile>::bind());
		// e.register_function("void LogToClipboard(int auto_open_depth)", ImBinder<&ImGui::LogToClipboard>::bind());
		// e.register_function("void LogFinish()", ImBinder<&ImGui::LogFinish>::bind());
		// e.register_function("void LogButtons()", ImBinder<&ImGui::LogButtons>::bind());
		// e.register_function("void LogText(const string& fmt)", ImBinder<&ImGui::LogText>::bind());
		// e.register_function("void LogTextV(const string& fmt, int args)", ImBinder<&ImGui::LogTextV>::bind());
		// e.register_function("bool BeginDragDropSource(int flags = 0)", ImBinder<&ImGui::BeginDragDropSource>::bind());
		// e.register_function("bool SetDragDropPayload(const string& type, Ptr<void> data, int sz, int cond = 0)", ImBinder<&ImGui::SetDragDropPayload>::bind());
		// e.register_function("void EndDragDropSource()", ImBinder<&ImGui::EndDragDropSource>::bind());
		// e.register_function("bool BeginDragDropTarget()", ImBinder<&ImGui::BeginDragDropTarget>::bind());
		// e.register_function("Ptr<ImGuiPayload> AcceptDragDropPayload(const string& type, int flags = 0)", ImBinder<&ImGui::AcceptDragDropPayload>::bind());
		// e.register_function("void EndDragDropTarget()", ImBinder<&ImGui::EndDragDropTarget>::bind());
		// e.register_function("Ptr<ImGuiPayload> GetDragDropPayload()", ImBinder<&ImGui::GetDragDropPayload>::bind());
		// e.register_function("void BeginDisabled(bool disabled = true)", ImBinder<&ImGui::BeginDisabled>::bind());
		// e.register_function("void EndDisabled()", ImBinder<&ImGui::EndDisabled>::bind());
		// e.register_function("void PushClipRect(const ImVec2 & clip_rect_min, const ImVec2 & clip_rect_max, bool intersect_with_current_clip_rect)", ImBinder<&ImGui::PushClipRect>::bind());
		// e.register_function("void PopClipRect()", ImBinder<&ImGui::PopClipRect>::bind());
		// e.register_function("void SetItemDefaultFocus()", ImBinder<&ImGui::SetItemDefaultFocus>::bind());
		// e.register_function("void SetKeyboardFocusHere(int offset = 0)", ImBinder<&ImGui::SetKeyboardFocusHere>::bind());
		// e.register_function("void SetNextItemAllowOverlap()", ImBinder<&ImGui::SetNextItemAllowOverlap>::bind());
		// e.register_function("bool IsItemHovered(int flags = 0)", ImBinder<&ImGui::IsItemHovered>::bind());
		e.register_function("bool IsItemActive()", ImBinder<&ImGui::IsItemActive>::bind());
		e.register_function("bool IsItemFocused()", ImBinder<&ImGui::IsItemFocused>::bind());
		e.register_function("bool IsItemClicked(int mouse_button = 0)", ImBinder<&ImGui::IsItemClicked>::bind());
		e.register_function("bool IsItemVisible()", ImBinder<&ImGui::IsItemVisible>::bind());
		e.register_function("bool IsItemEdited()", ImBinder<&ImGui::IsItemEdited>::bind());
		e.register_function("bool IsItemActivated()", ImBinder<&ImGui::IsItemActivated>::bind());
		e.register_function("bool IsItemDeactivated()", ImBinder<&ImGui::IsItemDeactivated>::bind());
		e.register_function("bool IsItemDeactivatedAfterEdit()", ImBinder<&ImGui::IsItemDeactivatedAfterEdit>::bind());
		e.register_function("bool IsItemToggledOpen()", ImBinder<&ImGui::IsItemToggledOpen>::bind());
		e.register_function("bool IsAnyItemHovered()", ImBinder<&ImGui::IsAnyItemHovered>::bind());
		e.register_function("bool IsAnyItemActive()", ImBinder<&ImGui::IsAnyItemActive>::bind());
		e.register_function("bool IsAnyItemFocused()", ImBinder<&ImGui::IsAnyItemFocused>::bind());
		e.register_function("uint GetItemID()", ImBinder<&ImGui::GetItemID>::bind());
		e.register_function("ImVec2 GetItemRectMin()", ImBinder<&ImGui::GetItemRectMin>::bind());
		e.register_function("ImVec2 GetItemRectMax()", ImBinder<&ImGui::GetItemRectMax>::bind());
		e.register_function("ImVec2 GetItemRectSize()", ImBinder<&ImGui::GetItemRectSize>::bind());
		// e.register_function("Ptr<ImGuiViewport> GetMainViewport()", ImBinder<&ImGui::GetMainViewport>::bind());
		// e.register_function("Ptr<ImDrawList> GetBackgroundDrawList()", ImBinder<overload_of<ImDrawList *>(&ImGui::GetBackgroundDrawList)>::bind());
		// e.register_function("Ptr<ImDrawList> GetForegroundDrawList()", ImBinder<overload_of<ImDrawList *>(&ImGui::GetForegroundDrawList)>::bind());
		// e.register_function("Ptr<ImDrawList> GetBackgroundDrawList(Ptr<ImGuiViewport> viewport)", ImBinder<overload_of<ImDrawList *, ImGuiViewport *>(&ImGui::GetBackgroundDrawList)>::bind());
		// e.register_function("Ptr<ImDrawList> GetForegroundDrawList(Ptr<ImGuiViewport> viewport)", ImBinder<overload_of<ImDrawList *, ImGuiViewport *>(&ImGui::GetForegroundDrawList)>::bind());
		// e.register_function("bool IsRectVisible(const ImVec2 & size)", ImBinder<overload_of<bool, const ImVec2 &>(&ImGui::IsRectVisible)>::bind());
		// e.register_function("bool IsRectVisible(const ImVec2 & rect_min, const ImVec2 & rect_max)", ImBinder<overload_of<bool, const ImVec2 &, const ImVec2 &>(&ImGui::IsRectVisible)>::bind());
		// e.register_function("double GetTime()", ImBinder<&ImGui::GetTime>::bind());
		// e.register_function("int GetFrameCount()", ImBinder<&ImGui::GetFrameCount>::bind());
		// e.register_function("Ptr<ImDrawListSharedData> GetDrawListSharedData()", ImBinder<&ImGui::GetDrawListSharedData>::bind());
		// e.register_function("string GetStyleColorName(int idx)", ImBinder<&ImGui::GetStyleColorName>::bind());
		// e.register_function("void SetStateStorage(Ptr<ImGuiStorage> storage)", ImBinder<&ImGui::SetStateStorage>::bind());
		// e.register_function("Ptr<ImGuiStorage> GetStateStorage()", ImBinder<&ImGui::GetStateStorage>::bind());
		// e.register_function("ImVec2 CalcTextSize(const string& text, const string& text_end = nullptr, bool hide_text_after_double_hash = false, float wrap_width)", ImBinder<&ImGui::CalcTextSize>::bind());
		// e.register_function("ImVec4 ColorConvertU32ToFloat4(unsigned int in)", ImBinder<&ImGui::ColorConvertU32ToFloat4>::bind());
		// e.register_function("unsigned int ColorConvertFloat4ToU32(const ImVec4 & in)", ImBinder<&ImGui::ColorConvertFloat4ToU32>::bind());
		// e.register_function("void ColorConvertRGBtoHSV(float r, float g, float b, float & out_h, float & out_s, float & out_v)", ImBinder<&ImGui::ColorConvertRGBtoHSV>::bind());
		// e.register_function("void ColorConvertHSVtoRGB(float h, float s, float v, float & out_r, float & out_g, float & out_b)", ImBinder<&ImGui::ColorConvertHSVtoRGB>::bind());
		// e.register_function("bool IsKeyDown(ImGuiKey key)", ImBinder<&ImGui::IsKeyDown>::bind());
		// e.register_function("bool IsKeyPressed(ImGuiKey key, bool repeat = true)", ImBinder<&ImGui::IsKeyPressed>::bind());
		// e.register_function("bool IsKeyReleased(ImGuiKey key)", ImBinder<&ImGui::IsKeyReleased>::bind());
		// e.register_function("bool IsKeyChordPressed(int key_chord)", ImBinder<&ImGui::IsKeyChordPressed>::bind());
		// e.register_function("int GetKeyPressedAmount(ImGuiKey key, float repeat_delay, float rate)", ImBinder<&ImGui::GetKeyPressedAmount>::bind());
		// e.register_function("string GetKeyName(ImGuiKey key)", ImBinder<&ImGui::GetKeyName>::bind());
		// e.register_function("void SetNextFrameWantCaptureKeyboard(bool want_capture_keyboard)", ImBinder<&ImGui::SetNextFrameWantCaptureKeyboard>::bind());
		// e.register_function("bool IsMouseDown(int button)", ImBinder<&ImGui::IsMouseDown>::bind());
		// e.register_function("bool IsMouseClicked(int button, bool repeat = false)", ImBinder<&ImGui::IsMouseClicked>::bind());
		// e.register_function("bool IsMouseReleased(int button)", ImBinder<&ImGui::IsMouseReleased>::bind());
		// e.register_function("bool IsMouseDoubleClicked(int button)", ImBinder<&ImGui::IsMouseDoubleClicked>::bind());
		// e.register_function("int GetMouseClickedCount(int button)", ImBinder<&ImGui::GetMouseClickedCount>::bind());
		// e.register_function("bool IsMouseHoveringRect(const ImVec2 & r_min, const ImVec2 & r_max, bool clip = true)", ImBinder<&ImGui::IsMouseHoveringRect>::bind());
		// e.register_function("bool IsMousePosValid(Ptr<ImVec2> mouse_pos = nullptr)", ImBinder<&ImGui::IsMousePosValid>::bind());
		// e.register_function("bool IsAnyMouseDown()", ImBinder<&ImGui::IsAnyMouseDown>::bind());
		// e.register_function("ImVec2 GetMousePos()", ImBinder<&ImGui::GetMousePos>::bind());
		// e.register_function("ImVec2 GetMousePosOnOpeningCurrentPopup()", ImBinder<&ImGui::GetMousePosOnOpeningCurrentPopup>::bind());
		// e.register_function("bool IsMouseDragging(int button, float lock_threshold)", ImBinder<&ImGui::IsMouseDragging>::bind());
		// e.register_function("ImVec2 GetMouseDragDelta(int button = 0, float lock_threshold)", ImBinder<&ImGui::GetMouseDragDelta>::bind());
		// e.register_function("void ResetMouseDragDelta(int button = 0)", ImBinder<&ImGui::ResetMouseDragDelta>::bind());
		// e.register_function("int GetMouseCursor()", ImBinder<&ImGui::GetMouseCursor>::bind());
		// e.register_function("void SetMouseCursor(int cursor_type)", ImBinder<&ImGui::SetMouseCursor>::bind());
		// e.register_function("void SetNextFrameWantCaptureMouse(bool want_capture_mouse)", ImBinder<&ImGui::SetNextFrameWantCaptureMouse>::bind());
		// e.register_function("string GetClipboardText()", ImBinder<&ImGui::GetClipboardText>::bind());
		// e.register_function("void SetClipboardText(const string& text)", ImBinder<&ImGui::SetClipboardText>::bind());
		// e.register_function("void LoadIniSettingsFromDisk(const string& ini_filename)", ImBinder<&ImGui::LoadIniSettingsFromDisk>::bind());
		// e.register_function("void LoadIniSettingsFromMemory(const string& ini_data, int ini_size = 0)", ImBinder<&ImGui::LoadIniSettingsFromMemory>::bind());
		// e.register_function("void SaveIniSettingsToDisk(const string& ini_filename)", ImBinder<&ImGui::SaveIniSettingsToDisk>::bind());
		// e.register_function("string SaveIniSettingsToMemory(Ptr<int> out_ini_size = nullptr)", ImBinder<&ImGui::SaveIniSettingsToMemory>::bind());
		// e.register_function("void DebugTextEncoding(const string& text)", ImBinder<&ImGui::DebugTextEncoding>::bind());
		// e.register_function("void DebugFlashStyleColor(int idx)", ImBinder<&ImGui::DebugFlashStyleColor>::bind());
		// e.register_function("bool DebugCheckVersionAndDataLayout(const string& version_str, int sz_io, int sz_style, int sz_vec2, int sz_vec4, int sz_drawvert, int sz_drawidx)", ImBinder<&ImGui::DebugCheckVersionAndDataLayout>::bind());
		// e.register_function("void SetAllocatorFunctions(void *(*)(int, void *) alloc_func, void (*)(void *, void *) free_func, Ptr<void> user_data = nullptr)", ImBinder<&ImGui::SetAllocatorFunctions>::bind());
		// e.register_function("void GetAllocatorFunctions(Ptr<*)> p_alloc_func, Ptr<*)> p_free_func, Ptr<*> p_user_data)", ImBinder<&ImGui::GetAllocatorFunctions>::bind());
		// e.register_function("Ptr<void> MemAlloc(int size)", ImBinder<&ImGui::MemAlloc>::bind());
		// e.register_function("void MemFree(Ptr<void> ptr)", ImBinder<&ImGui::MemFree>::bind());
		// e.register_function("ImGuiPlatformIO & GetPlatformIO()", ImBinder<&ImGui::GetPlatformIO>::bind());
		// e.register_function("void UpdatePlatformWindows()", ImBinder<&ImGui::UpdatePlatformWindows>::bind());
		// e.register_function("void RenderPlatformWindowsDefault(Ptr<void> platform_render_arg = nullptr, Ptr<void> renderer_render_arg = nullptr)", ImBinder<&ImGui::RenderPlatformWindowsDefault>::bind());
		// e.register_function("void DestroyPlatformWindows()", ImBinder<&ImGui::DestroyPlatformWindows>::bind());
		// e.register_function("Ptr<ImGuiViewport> FindViewportByID(unsigned int id)", ImBinder<&ImGui::FindViewportByID>::bind());
		// e.register_function("Ptr<ImGuiViewport> FindViewportByPlatformHandle(Ptr<void> platform_handle)", ImBinder<&ImGui::FindViewportByPlatformHandle>::bind());
		// e.register_function("ImGuiKey GetKeyIndex(ImGuiKey key)", ImBinder<&ImGui::GetKeyIndex>::bind());
		// e.register_function("bool BeginChildFrame(unsigned int id, const ImVec2 & size, int window_flags = 0)", ImBinder<&ImGui::BeginChildFrame>::bind());
		// e.register_function("void EndChildFrame()", ImBinder<&ImGui::EndChildFrame>::bind());
		// e.register_function("void ShowStackToolWindow(Ptr<bool> p_open = nullptr)", ImBinder<&ImGui::ShowStackToolWindow>::bind());
		// e.register_function("bool ListBox(const string& label, Ptr<int> current_item, Ptr<**)> old_callback, Ptr<void> user_data, int items_count, int height_in_items)", ImBinder<overload_of<bool, const char *, int *, bool (*)(void *, int, const char **), void *, int, int>(&ImGui::ListBox)>::bind());
		// e.register_function("bool Combo(const string& label, Ptr<int> current_item, Ptr<**)> old_callback, Ptr<void> user_data, int items_count, int popup_max_height_in_items)", ImBinder<overload_of<bool, const char *, int *, bool (*)(void *, int, const char **), void *, int, int>(&ImGui::Combo)>::bind());
		// e.register_function("void SetItemAllowOverlap()", ImBinder<&ImGui::SetItemAllowOverlap>::bind());
		// e.register_function("void PushAllowKeyboardFocus(bool tab_stop)", ImBinder<&ImGui::PushAllowKeyboardFocus>::bind());
		// e.register_function("void PopAllowKeyboardFocus()", ImBinder<&ImGui::PopAllowKeyboardFocus>::bind());
		// e.register_function("bool ImageButton(ImTextureID user_texture_id, const ImVec2 & size, const ImVec2 & uv0, const ImVec2 & uv1, int frame_padding, const ImVec4 & bg_col, const ImVec4 & tint_col)", ImBinder<overload_of<bool, ImTextureID, const ImVec2 &, const ImVec2 &, const ImVec2 &, int, const ImVec4 &, const ImVec4 &>(&ImGui::ImageButton)>::bind());
		// e.register_function("void CaptureKeyboardFromApp(bool want_capture_keyboard = true)", ImBinder<&ImGui::CaptureKeyboardFromApp>::bind());
		// e.register_function("void CaptureMouseFromApp(bool want_capture_mouse = true)", ImBinder<&ImGui::CaptureMouseFromApp>::bind());
		// e.register_function("void CalcListClipping(int items_count, float items_height, Ptr<int> out_items_display_start, Ptr<int> out_items_display_end)", ImBinder<&ImGui::CalcListClipping>::bind());
		// clang-format on
	}

	static void register_imgui()
	{
		register_enums();
		register_structures();
		register_functions();
	}

	static ReflectionInitializeController on_init(register_imgui, "ImGui");
}// namespace Engine
