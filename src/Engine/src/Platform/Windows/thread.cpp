#include <windows.h>
#define MAX_THREAD_NAME_LENGTH 32

#include "../Shared/thread.inl"

namespace Engine
{
    void Thread::sleep_for(float seconds)
    {
        Sleep(static_cast<DWORD>(seconds * 1000.f));
    }
}// namespace Engine
