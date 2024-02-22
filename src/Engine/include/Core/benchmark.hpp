#pragma once
#include <Core/engine_types.hpp>
#include <Core/logger.hpp>
#include <chrono>
#include <string>

namespace Engine
{
#define BENCH_CODE(code)                                                                                               \
    {                                                                                                                  \
        BenchMark m_bench;                                                                                            \
        {                                                                                                              \
            code                                                                                                       \
        }                                                                                                              \
    }
    template<typename duration = std::chrono::microseconds>
    class BenchMark final
    {
    private:
        int_t m_line;
        String m_file;
        String m_function;
        std::chrono::steady_clock::time_point m_start;
        bool m_enable_log = true;

    public:
        String message;
        BenchMark(int_t line = __builtin_LINE(), const String& file = __builtin_FILE(),
                  const String& function = __builtin_FUNCTION())
            : m_line(line), m_file(file), m_function(function), m_start(std::chrono::steady_clock::now())
        {}


        std::size_t time()
        {
            return std::chrono::duration_cast<duration>(std::chrono::steady_clock::now() - m_start).count();
        }

        BenchMark& log_status(bool flag)
        {
            m_enable_log = flag;
            return *this;
        }

        bool log_status() const
        {
            return m_enable_log;
        }

        ~BenchMark()
        {
            if (m_enable_log)
                info_log("BenchMark", "%s: %d: %s(): %s %d", m_file.c_str(), m_line, m_function.c_str(), message.c_str(),
                            (int) time());
        }
    };
}// namespace Engine
