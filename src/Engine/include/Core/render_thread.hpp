#pragma once
#include <Core/thread.hpp>


namespace Engine
{
    ENGINE_EXPORT Thread* render_thread();

    template<typename Callable>
    FORCE_INLINE void call_in_render_thread(Callable&& callable)
    {
        struct Command : public ExecutableObject {
            Callable m_callable;

            Command(Callable&& callable) : m_callable(std::forward<Callable>(callable))
            {}

            int_t execute() override
            {
                m_callable();
                return sizeof(Command);
            }
        };

        Thread* rt = render_thread();
        if (Thread::this_thread() == rt)
        {
            callable();
        }
        else
        {
            rt->insert_new_task<Command>(std::forward<Callable>(callable));
        }
    }
}// namespace Engine
