#pragma once
#include <Core/engine_types.hpp>

namespace Trinex
{
	struct OperationSystemType {
		enum Enum
		{
			Linux,
			Windows,
			Android,
		};

		trinex_enum_struct(OperationSystemType);
		trinex_enum(OperationSystemType);
	};

	struct PhysicalSizeMetric {
		enum Enum
		{
			Inch,
			Сentimeters,
		};

		trinex_enum_struct(PhysicalSizeMetric);
		trinex_enum(PhysicalSizeMetric);
	};

	struct WindowAttribute {
		enum Enum : u64
		{
			Undefined = 0,

			Fullscreen = 1ull << 0,
			Hidden     = 1ull << 1,
			Borderless = 1ull << 2,
			Resizable  = 1ull << 3,

			Minimized = 1ull << 4,
			Maximized = 1ull << 5,
			Occluded  = 1ull << 6,

			InputFocus = 1ull << 7,
			MouseFocus = 1ull << 8,

			MouseGrabbed      = 1ull << 9,
			KeyboardGrabbed   = 1ull << 10,
			MouseCapture      = 1ull << 11,
			MouseRelativeMode = 1ull << 12,

			AlwaysOnTop      = 1ull << 13,
			HighPixelDensity = 1ull << 14,
			Transparent      = 1ull << 15,
			NotFocusable     = 1ull << 16,

			Modal     = 1ull << 17,
			Utility   = 1ull << 18,
			Tooltip   = 1ull << 19,
			PopupMenu = 1ull << 20,

			External = 1ull << 21,

			// State controlled by the window manager / OS.
			ReadOnly = Occluded | InputFocus | MouseFocus | MouseCapture,

			// Can be directly changed at runtime.
			ReadWrite = Fullscreen | Hidden | Borderless | Resizable | MouseGrabbed | KeyboardGrabbed | MouseRelativeMode |
			            AlwaysOnTop | NotFocusable | Modal,

			// Runtime state changed through commands:
			// minimize(), maximize(), restore().
			Command = Minimized | Maximized,

			// Determined when the window is created.
			CreateOnly = HighPixelDensity | Transparent | Utility | Tooltip | PopupMenu | External,

			// Input-related state.
			Input = InputFocus | MouseFocus | MouseGrabbed | KeyboardGrabbed | MouseCapture | MouseRelativeMode,

			// Window presentation/state.
			State = Fullscreen | Hidden | Minimized | Maximized | Occluded,

			// Window configuration/decorations.
			Configuration = Borderless | Resizable | AlwaysOnTop | NotFocusable | Modal,

			// Special window roles.
			Role = Utility | Tooltip | PopupMenu,

			// Can potentially change after creation.
			Runtime = ReadOnly | ReadWrite | Command,

			// Can be changed by the application after creation.
			Mutable = ReadWrite | Command,

			// Cannot be directly changed by Window API.
			Immutable = ReadOnly | CreateOnly,

			All = ReadOnly | ReadWrite | Command | CreateOnly,
		};

		trinex_bitfield_enum_struct(WindowAttribute, u64);
	};

	struct CursorMode {
		enum Enum : EnumerateType
		{
			Normal,
			Hidden,
		};

		trinex_enum_struct(CursorMode);
		trinex_enum(CursorMode);
	};

	struct Orientation {
		enum Enum : EnumerateType
		{
			Landscape        = 0,
			LandscapeFlipped = 1,
			Portrait         = 2,
			PortraitFlipped  = 3,
		};

		trinex_enum_struct(Orientation);
		trinex_enum(Orientation);
	};

	struct MessageBoxType {
		enum Enum
		{
			Error,
			Warning,
			Info,
		};

		trinex_enum_struct(MessageBoxType);
		trinex_enum(MessageBoxType);
	};

	struct ArchiveFlags {
		enum Enum : EnumerateType
		{
			Undefined        = 0,
			SkipObjectSearch = BIT(0),
			IsCopyProcess    = BIT(1),
		};

		trinex_bitfield_enum_struct(ArchiveFlags, EnumerateType);
	};

	struct IOWhence {
		enum Enum : u8
		{
			Current = 0,
			Begin   = 1,
			End     = 2,
		};

		trinex_enum_struct(IOWhence);
	};

	struct IOMode {
		enum Enum : u8
		{
			Read  = 0,
			Write = 1,
		};

		trinex_enum_struct(IOMode);
	};

	struct SplashTextType {
		enum Enum : EnumerateType
		{
			StartupProgress = 0,
			VersionInfo     = 1,
			CopyrightInfo   = 2,
			GameName        = 3,
			Count           = 4,
		};

		trinex_enum_struct(SplashTextType);
		trinex_enum(SplashTextType);
	};
}// namespace Trinex
