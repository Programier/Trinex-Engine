#define MAX_THREAD_NAME_LENGTH 16

#include "../Shared/thread.inl"

namespace Engine
{
    void Thread::sleep_for(float seconds)
    {
        usleep(static_cast<__useconds_t>(seconds * 1000000.f));
    }
}// namespace Engine
