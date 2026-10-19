#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/array.hpp>

namespace Trinex
{
	class trinex_api StackTrace final
	{
	public:
		struct FunctionInfo {
			String filename;
			String symbol_name;
			void* func_address = nullptr;
		};

	private:
		Array<FunctionInfo> m_callstack;

	public:
		StackTrace(u32 skip = 1);
		const Array<FunctionInfo>& callstack() const;
		String to_string(bool with_filename = false, const char* line_sep = "\n") const;
	};
}// namespace Trinex
