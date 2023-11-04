#include <Core/etl/ring_buffer.hpp>
#include <Core/executable_object.hpp>
#include <Core/logger.hpp>
#include <Platform/thread.hpp>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <thread>


namespace Engine
{
    static constexpr inline size_t max_thread_name_length = 16;

#include "../Shared/thread.inl"

    void Thread::sleep_for(float seconds)
    {
        usleep(static_cast<__useconds_t>(seconds * 1000000.f));
    }
}// namespace Engine
