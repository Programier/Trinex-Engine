#pragma once
#include <Core/filesystem/enums.hpp>

namespace Trinex::VFS
{
	struct FileStat {
		FileType type     = FileType::Undefined;
		u64 size          = 0;
		u64 accessed_time = 0;
		u64 modified_time = 0;
		u64 created_time  = 0;
	};
}// namespace Trinex::VFS
