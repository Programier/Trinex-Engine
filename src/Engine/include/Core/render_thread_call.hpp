#pragma once
#include <Core/engine.hpp>
#include <Core/thread.hpp>


namespace Engine
{
    template<typename Callable>
    FORCE_INLINE void call_in_render_thread(Callable&& callable)
    {
        struct Command : public ExecutableObject {
            Callable _M_callable;

            Command(Callable&& callable) : _M_callable(std::forward<Callable>(callable))
            {}

            int_t execute() override
            {
                _M_callable();
                return sizeof(Command);
            }
        };

        Thread* render_thread = engine_instance->thread(ThreadType::RenderThread);
        if (Thread::this_thread() == render_thread)
        {
            callable();
        }
        else
        {
            render_thread->insert_new_task<Command>(std::forward<Callable>(callable));
        }
    }
}// namespace Engine
