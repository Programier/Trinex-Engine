#pragma once

namespace Trinex::VFS
{
	struct AccessFlags {
		enum Enum : u8
		{
			Read      = BIT(0),
			Write     = BIT(1),
			ReadWrite = Read | Write,
			Append    = BIT(2) | Write,
			Recursive = BIT(3),
		};

		trinex_bitfield_enum_struct(AccessFlags, u8);
	};

	struct FileType {
		enum Enum : u8
		{
			Undefined,
			Regular,
			Directory,
			Other,
		};
		trinex_enum_struct(FileType);
	};

	struct WalkFlags {
		enum Enum : u8
		{
			Undefined   = 0,
			Files       = BIT(0),
			Directories = BIT(1),
			Recursive   = BIT(2),
			Default     = Files | Directories,
		};

		trinex_bitfield_enum_struct(WalkFlags, u8);
	};

	struct WalkResult {
		enum Enum : u8
		{
			Continue = 0,
			Skip     = 1,
			Stop     = 2,
		};
		trinex_enum_struct(WalkResult);
	};
}// namespace Trinex::VFS
