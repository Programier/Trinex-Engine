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
		enum Enum : u16
		{
			Undefined       = 0,
			Resizable       = 1 << 0,
			FullScreen      = 1 << 1,
			Shown           = 1 << 2,
			Hidden          = 1 << 3,
			BorderLess      = 1 << 4,
			MouseFocus      = 1 << 5,
			InputFocus      = 1 << 6,
			InputGrabbed    = 1 << 7,
			Minimized       = 1 << 8,
			Maximized       = 1 << 9,
			MouseCapture    = 1 << 10,
			MouseGrabbed    = 1 << 11,
			KeyboardGrabbed = 1 << 12,
			Vsync           = 1 << 13,
		};

		trinex_bitfield_enum_struct(WindowAttribute, u16);
		trinex_enum(WindowAttribute);
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

	struct SerializationFlags {
		enum Enum : EnumerateType
		{
			None             = 0,
			SkipObjectSearch = BIT(0),
			IsCopyProcess    = BIT(1),
		};

		trinex_bitfield_enum_struct(SerializationFlags, EnumerateType);
	};

	struct BufferSeekDir {
		enum Enum : EnumerateType
		{
			Current = 0,
			Begin   = 1,
			End     = 2,
		};

		trinex_enum_struct(BufferSeekDir);
	};

	struct FileOpenMode {
		enum Enum : u8
		{
			Read      = BIT(0),
			Write     = BIT(1),
			ReadWrite = Read | Write,
			Append    = BIT(2) | Write,
		};

		trinex_bitfield_enum_struct(FileOpenMode, u8);
	};

	using FileSeekDir = BufferSeekDir;

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
