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
	
	struct EnvironmentVariable{
		
	};
}// namespace Trinex::Platform
