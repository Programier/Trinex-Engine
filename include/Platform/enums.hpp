#pragma once

namespace Trinex::Platform
{
	struct SystemType {
		enum Enum : u8
		{
			Unknown = 0,

			// Desktop
			Linux,
			Windows,
			MacOS,
			FreeBSD,

			// Mobile
			Android,
			IOS,
			IPadOS,

			// Web
			Web,

			// VR / AR
			VisionOS,
			Quest,
			HorizonOS,
			Pico,

			// Consoles / handhelds
			SteamOS,
			XboxOne,
			XboxSeries,
			PlayStation4,
			PlayStation5,
			NintendoSwitch,
			NintendoSwitch2,
		};

		trinex_enum_struct(SystemType);
	};

	struct Architecture {
		enum Enum : u8
		{
			Unknown = 0,
			X86,
			X86_64,
			ARM,
			ARM64,
			RiscV64,
		};

		trinex_enum_struct(Architecture);
	};

	struct CPUFeature {
		enum Enum : u64
		{
			Undefined = 0,
			SSE       = BIT(0),
			SSE2      = BIT(1),
			SSE3      = BIT(2),
			SSSE3     = BIT(3),
			SSE41     = BIT(4),
			SSE42     = BIT(5),
			AVX       = BIT(6),
			AVX2      = BIT(7),
			AVX512F   = BIT(8),
			AES       = BIT(9),
			SHA       = BIT(10),
			FMA       = BIT(11),
			NEON      = BIT(12),
			CRC32     = BIT(13),
		};

		trinex_bitfield_enum_struct(CPUFeature, u64);
	};

	struct MemoryProtection {
		enum Enum : u8
		{
			NoAccess         = 0,
			Read             = BIT(0),
			Write            = BIT(1),
			Execute          = BIT(2),
			ReadWrite        = Read | Write,
			ReadExecute      = Read | Execute,
			ReadWriteExecute = Read | Write | Execute,
		};

		trinex_bitfield_enum_struct(MemoryProtection, u8);
	};

	struct VirtualMemoryFlags {
		enum Enum : u8
		{
			Undefined      = 0,
			LargePage      = BIT(0),
			Shared         = BIT(1),
			Stack          = BIT(2),
			NoReserve      = BIT(3),
			Populate       = BIT(4),
			FixedNoReplace = BIT(5),
		};

		trinex_bitfield_enum_struct(VirtualMemoryFlags, u8);
	};

	struct FileAccess {
		enum Enum : u8
		{
			Undefined = 0,
			Read      = BIT(0),
			Write     = BIT(1),
			Execute   = BIT(2),
			ReadWrite = Read | Write,
		};

		trinex_bitfield_enum_struct(FileAccess, u8);
	};

	struct FileShare {
		enum Enum : u8
		{
			None   = 0,
			Read   = BIT(0),
			Write  = BIT(1),
			Delete = BIT(2),
			All    = Read | Write | Delete,
		};

		trinex_bitfield_enum_struct(FileShare, u8);
	};

	struct FileCreateMode {
		enum Enum : u8
		{
			OpenExisting = 0,
			CreateNew,
			CreateAlways,
			OpenAlways,
			TruncateExisting,
		};

		trinex_enum_struct(FileCreateMode);
	};

	struct FileAttribute {
		enum Enum : u16
		{
			Undefined  = 0,
			File       = BIT(0),
			Directory  = BIT(1),
			Symlink    = BIT(2),
			Hidden     = BIT(3),
			ReadOnly   = BIT(4),
			Executable = BIT(5),
		};

		trinex_bitfield_enum_struct(FileAttribute, u16);
	};

	struct FileWatchEventType {
		enum Enum : u32
		{
			Undefined = 0,
			Created   = BIT(0),
			Removed   = BIT(1),
			Modified  = BIT(2),
			Renamed   = BIT(3),
			Metadata  = BIT(4),
			Any       = Created | Removed | Modified | Renamed | Metadata,
		};

		trinex_bitfield_enum_struct(FileWatchEventType, u32);
	};

	struct ProcessIO {
		enum Enum : u8
		{
			Undefined   = 0,
			Inherited   = 1,
			Application = 2,
			Redirect    = 3,
		};

		trinex_enum_struct(ProcessIO);
	};

	struct ThreadPriority {
		enum Enum : u8
		{
			Lowest = 0,
			BelowNormal,
			Normal,
			AboveNormal,
			Highest,
			TimeCritical,
		};

		trinex_enum_struct(ThreadPriority);
	};

	struct DialogResult {
		enum Enum : u8
		{
			Undefined = 0,
			Accepted,
			Rejected,
			Cancelled,
			Failed,
		};

		trinex_enum_struct(DialogResult);
	};

	struct DialogOption {
		enum Enum : u16
		{
			Undefined          = 0,
			AllowMultiple      = BIT(0),
			DirectoryOnly      = BIT(1),
			FileMustExist      = BIT(2),
			DirectoryMustExist = BIT(3),
			ShowHidden         = BIT(4),
			OverwritePrompt    = BIT(5),
			Modal              = BIT(6),
		};

		trinex_bitfield_enum_struct(DialogOption, u16);
	};

	struct NotificationType {
		enum Enum : u8
		{
			Message = 0,
			Info,
			Warning,
			Error,
			Success,
		};

		trinex_enum_struct(NotificationType);
	};

	struct NotificationPriority {
		enum Enum : u8
		{
			Low = 0,
			Normal,
			High,
			Critical,
		};

		trinex_enum_struct(NotificationPriority);
	};
}// namespace Trinex::Platform
