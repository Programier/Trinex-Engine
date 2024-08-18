#include <Core/demangle.hpp>
#include <Core/stacktrace.hpp>
#include <sstream>

#if PLATFORM_WINDOWS
#else
#include <dlfcn.h>
#include <execinfo.h>
#endif
namespace Engine
{

#if PLATFORM_WINDOWS
	StackTrace::StackTrace(uint_t skip)
	{}
#else
	StackTrace::StackTrace(uint_t skip)
	{
#if !PLATFORM_ANDROID || ANDROID_API >= 33
		void* callstack[2048];
		uint_t frames = backtrace(callstack, 2048);
		if (frames < skip)
		{
			return;
		}

		char** symbols = backtrace_symbols(callstack, frames);
		m_callstack.resize(frames - skip);

		for (uint_t i = skip; i < frames; ++i)
		{
			Dl_info info;
			if (dladdr(callstack[i], &info))
			{
				FunctionInfo& func_info = m_callstack[i - skip];
				func_info.filename		= info.dli_fname ? info.dli_fname : "Unknown File";
				func_info.func_address	= info.dli_saddr;
				func_info.symbol_name	= info.dli_sname ? Demangle::decode_name(info.dli_sname) : "Unknown Func";
			}
		}

		free(symbols);
#endif
	}
#endif

	const Vector<StackTrace::FunctionInfo>& StackTrace::callstack() const
	{
		return m_callstack;
	}

	String StackTrace::to_string(bool with_filename, const char* line_sep) const
	{
		std::stringstream ss;
		for (auto& info : m_callstack)
		{
			if (with_filename)
				ss << info.filename << ": ";
			ss << info.symbol_name << "[" << info.func_address << "]" << line_sep;
		}

		return ss.str();
	}

}// namespace Engine
