#pragma once
#include <Core/engine_types.hpp>


namespace Engine
{
    class ExecutableObject;

    class ENGINE_EXPORT Thread
    {
    public:
        using Task = ExecutableObject*;

    public:
        static Thread* new_thread(const String& name, size_t command_buffer_size = 2048);
        static Thread* this_thread();
        static void sleep_for(float seconds);

        virtual const String& name() const              = 0;
        virtual Thread& name(const String& thread_name) = 0;
        virtual bool has_unfinished_tasks() const       = 0;
        virtual bool is_thread_sleep() const            = 0;
        virtual bool is_busy() const                    = 0;
        virtual bool is_shutting_down() const           = 0;
        virtual Thread& wait_all()                      = 0;
        virtual Thread& push_task(Task task)            = 0;
        virtual Identifier id()                         = 0;
        virtual bool is_destroyable()                   = 0;

        virtual ~Thread() = default;
    };
}// namespace Engine
