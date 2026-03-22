#pragma once
#include <Core/engine_types.hpp>
#include <Core/math/vector.hpp>

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
		enum Enum : EnumerateType
		{
			None            = 0,
			Resizable       = 1,
			FullScreen      = 2,
			Shown           = 3,
			Hidden          = 4,
			BorderLess      = 5,
			MouseFocus      = 6,
			InputFocus      = 7,
			InputGrabbed    = 8,
			Minimized       = 9,
			Maximized       = 10,
			MouseCapture    = 12,
			MouseGrabbed    = 14,
			KeyboardGrabbed = 15,
		};

		trinex_enum_struct(WindowAttribute);
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
