#pragma once

namespace Trinex::VFS
{
	struct AccessFlags {
		enum Enum : u8
		{
			Read      = trinex_bit(0),
			Write     = trinex_bit(1),
			ReadWrite = Read | Write,
			Append    = trinex_bit(2) | Write,
			Recursive = trinex_bit(3),
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
			Files       = trinex_bit(0),
			Directories = trinex_bit(1),
			Recursive   = trinex_bit(2),
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
