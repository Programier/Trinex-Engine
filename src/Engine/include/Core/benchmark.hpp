#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/logger.hpp>
#include <chrono>
#include <string>

namespace Engine
{
#define BENCH_CODE(code)                                                                                               \
    {                                                                                                                  \
        BenchMark _M_bench;                                                                                            \
        {                                                                                                              \
            code                                                                                                       \
        }                                                                                                              \
    }
    template<typename duration = std::chrono::microseconds>
    class BenchMark final
    {
    private:
        int_t _M_line;
        String _M_file;
        String _M_function;
        std::chrono::steady_clock::time_point _M_start;
        bool _M_enable_log = true;

    public:
        String message;
        BenchMark(int_t line = __builtin_LINE(), const String& file = __builtin_FILE(),
                  const String& function = __builtin_FUNCTION())
            : _M_line(line), _M_file(file), _M_function(function), _M_start(std::chrono::steady_clock::now())
        {}


        std::size_t time()
        {
            return std::chrono::duration_cast<duration>(std::chrono::steady_clock::now() - _M_start).count();
        }

        BenchMark& log_status(bool flag)
        {
            _M_enable_log = flag;
            return *this;
        }

        bool log_status() const
        {
            return _M_enable_log;
        }

        ~BenchMark()
        {
            if (_M_enable_log)
                logger->log("%s: %d: %s(): %ls %d", _M_file.c_str(), _M_line, _M_function.c_str(), message.c_str(),
                            (int) time());
        }
    };
}// namespace Engine
