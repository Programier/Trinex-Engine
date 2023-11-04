#include <Core/etl/ring_buffer.hpp>
#include <Core/executable_object.hpp>
#include <Core/logger.hpp>
#include <Platform/thread.hpp>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <thread>
#include <windows.h>


namespace Engine
{
    static constexpr inline size_t max_thread_name_length           = 32;

#include "../Shared/thread.inl"

    void Thread::sleep_for(float seconds)
    {
        Sleep(static_cast<DWORD>(seconds * 1000.f));
    }
}// namespace Engine
