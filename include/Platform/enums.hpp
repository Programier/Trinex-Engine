#pragma once

namespace Trinex::Platform
{
	struct SystemType
	{
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

	struct Architecture
	{
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

	struct CPUFeature
	{
		enum Enum : u64
		{
			Undefined = 0,
			SSE       = trinex_bit(0),
			SSE2      = trinex_bit(1),
			SSE3      = trinex_bit(2),
			SSSE3     = trinex_bit(3),
			SSE41     = trinex_bit(4),
			SSE42     = trinex_bit(5),
			AVX       = trinex_bit(6),
			AVX2      = trinex_bit(7),
			AVX512F   = trinex_bit(8),
			AES       = trinex_bit(9),
			SHA       = trinex_bit(10),
			FMA       = trinex_bit(11),
			NEON      = trinex_bit(12),
			CRC32     = trinex_bit(13),
		};

		trinex_bitfield_enum_struct(CPUFeature, u64);
	};

	struct MemoryProtection
	{
		enum Enum : u8
		{
			NoAccess         = 0,
			Read             = trinex_bit(0),
			Write            = trinex_bit(1),
			Execute          = trinex_bit(2),
			ReadWrite        = Read | Write,
			ReadExecute      = Read | Execute,
			ReadWriteExecute = Read | Write | Execute,
		};

		trinex_bitfield_enum_struct(MemoryProtection, u8);
	};

	struct VirtualMemoryFlags
	{
		enum Enum : u8
		{
			Undefined      = 0,
			LargePage      = trinex_bit(0),
			Shared         = trinex_bit(1),
			Stack          = trinex_bit(2),
			NoReserve      = trinex_bit(3),
			Populate       = trinex_bit(4),
			FixedNoReplace = trinex_bit(5),
		};

		trinex_bitfield_enum_struct(VirtualMemoryFlags, u8);
	};

	struct FileAccess
	{
		enum Enum : u8
		{
			Undefined = 0,
			Read      = trinex_bit(0),
			Write     = trinex_bit(1),
			Execute   = trinex_bit(2),
			ReadWrite = Read | Write,
		};

		trinex_bitfield_enum_struct(FileAccess, u8);
	};

	struct FileShare
	{
		enum Enum : u8
		{
			None   = 0,
			Read   = trinex_bit(0),
			Write  = trinex_bit(1),
			Delete = trinex_bit(2),
			All    = Read | Write | Delete,
		};

		trinex_bitfield_enum_struct(FileShare, u8);
	};

	struct FileCreateMode
	{
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

	struct FileAttribute
	{
		enum Enum : u16
		{
			Undefined  = 0,
			File       = trinex_bit(0),
			Directory  = trinex_bit(1),
			Symlink    = trinex_bit(2),
			Hidden     = trinex_bit(3),
			ReadOnly   = trinex_bit(4),
			Executable = trinex_bit(5),
		};

		trinex_bitfield_enum_struct(FileAttribute, u16);
	};

	struct FileWatchEventType
	{
		enum Enum : u32
		{
			Undefined = 0,
			Created   = trinex_bit(0),
			Removed   = trinex_bit(1),
			Modified  = trinex_bit(2),
			Renamed   = trinex_bit(3),
			Metadata  = trinex_bit(4),
			Any       = Created | Removed | Modified | Renamed | Metadata,
		};

		trinex_bitfield_enum_struct(FileWatchEventType, u32);
	};

	struct ProcessIO
	{
		enum Enum : u8
		{
			Undefined   = 0,
			Inherited   = 1,
			Application = 2,
			Redirect    = 3,
		};

		trinex_enum_struct(ProcessIO);
	};

	struct ThreadPriority
	{
		enum Enum : u8
		{
			Low      = 0,
			Normal   = 1,
			High     = 2,
			Critical = 3,
		};

		trinex_enum_struct(ThreadPriority);
	};

	struct ThreadState
	{
		enum Enum : u8
		{
			Undefined = 0,
			Alive     = 1,
			Detached  = 2,
			Complete  = 3,
		};

		trinex_enum_struct(ThreadState);
	};

	struct DialogStatus
	{
		enum Enum : u8
		{
			Undefined = 0,
			Accepted,
			Rejected,
			Cancelled,
			Failed,
		};

		trinex_enum_struct(DialogStatus);
	};
} // namespace Trinex::Platform
