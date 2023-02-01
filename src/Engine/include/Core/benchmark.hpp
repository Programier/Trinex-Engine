#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/logger.hpp>
#include <chrono>
#include <string>

namespace Engine
{
#define BENCH_CODE(code) {BenchMark _M_bench; {code} }
    template<typename duration = std::chrono::microseconds>
    class BenchMark final
    {
    private:
        int_t _M_line;
        std::string _M_file;
        std::string _M_function;
        std::chrono::steady_clock::time_point _M_start;

    public:
        String message;
        BenchMark(int_t line = __builtin_LINE(), const std::string& file = __builtin_FILE(),
                  const std::string& function = __builtin_FUNCTION())
            : _M_line(line), _M_file(file), _M_function(function), _M_start(std::chrono::steady_clock::now())
        {}

        ~BenchMark()
        {
            logger->log("%s: %d: %s(): %ls %zu", _M_file.c_str(), _M_line, _M_function.c_str(), message.c_str(),
                        std::chrono::duration_cast<duration>(std::chrono::steady_clock::now() - _M_start).count());
        }
    };
}// namespace Engine
