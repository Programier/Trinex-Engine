#pragma once
#include <Core/etl/string.hpp>

namespace Trinex::Platform
{
	using ProcessId    = u64;
	using ThreadId     = u64;
	using FileOffset   = i64;
	using FilePosition = u64;

	struct Result {
		bool success    = false;
		u32 error_code  = 0;
		const char* api = nullptr;

		FORCE_INLINE explicit operator bool() const { return success; }
	};

	struct Version {
		u32 major = 0;
		u32 minor = 0;
		u32 patch = 0;
		u32 build = 0;
	};

	struct EnvironmentVariable {
		String name;
		String value;
	};
}// namespace Trinex::Platform
