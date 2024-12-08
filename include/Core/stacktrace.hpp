#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>

namespace Engine
{
	class ENGINE_EXPORT StackTrace final
	{
	public:
		struct FunctionInfo {
			String filename;
			String symbol_name;
			void* func_address = nullptr;
		};

	private:
		Vector<FunctionInfo> m_callstack;

	public:
		StackTrace(uint_t skip = 1);
		const Vector<FunctionInfo>& callstack() const;
		String to_string(bool with_filename = false, const char* line_sep = "\n") const;
	};
}// namespace Engine
