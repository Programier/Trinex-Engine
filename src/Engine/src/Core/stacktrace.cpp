#include <Core/demangle.hpp>
#include <Core/stacktrace.hpp>
#include <dlfcn.h>
#include <execinfo.h>
#include <sstream>

namespace Engine
{

    StackTrace::StackTrace(uint_t skip)
    {
        void* callstack[2048];
        uint_t frames = backtrace(callstack, 2048);
        if (frames < skip)
        {
            return;
        }

        char** symbols = backtrace_symbols(callstack, frames);
        _M_callstack.resize(frames - skip);

        for (uint_t i = skip; i < frames; ++i)
        {
            Dl_info info;
            if (dladdr(callstack[i], &info))
            {
                FunctionInfo& func_info = _M_callstack[i - skip];
                func_info.filename      = info.dli_fname ? info.dli_fname : "Unknown File";
                func_info.func_address  = info.dli_saddr;
                func_info.symbol_name   = info.dli_sname ? Demangle::decode_name(info.dli_sname) : "Unknown Func";
            }
        }

        free(symbols);
    }

    const Vector<StackTrace::FunctionInfo>& StackTrace::callstack() const
    {
        return _M_callstack;
    }

    String StackTrace::to_string(bool with_filename, const char* line_sep) const
    {
        std::stringstream ss;
        for (auto& info : _M_callstack)
        {
            if (with_filename)
                ss << info.filename << ": ";
            ss << info.symbol_name << "[" << info.func_address << "]" << line_sep;
        }

        return ss.str();
    }

}// namespace Engine