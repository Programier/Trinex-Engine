#pragma once

#include <Core/engine_types.hpp>
#include <condition_variable>
#include <future>
#include <Core/executable_object.hpp>
#include <Core/etl/ring_buffer.hpp>

namespace Engine
{
    class ENGINE_EXPORT Thread final
    {
    public:
        using Task = ExecutableObject*;

#if PLATFORM_WINDOWS
        static constexpr size_t max_thread_name_length = 31;
#else
        static constexpr size_t max_thread_name_length = 15;
#endif

    private:
        std::condition_variable _M_cv;
        std::condition_variable _M_cv_for_wait;
        mutable std::mutex _M_mutex;
        mutable std::mutex _M_mutex2;
        mutable std::mutex _M_mutex_for_wait;

        String _M_name;

        RingBuffer<Task> _M_tasks;

        std::thread* _M_thread = nullptr;

        bool _M_is_shutting_down      = false;
        bool _M_is_thread_sleep       = false;

        bool platform_thread_change_name(const char* name);

        Thread& on_task_pushed();
        static void thread_loop(Thread* self);
        Task next_task();

    public:
        Thread(size_t command_buffer_size = 100);
        Thread(const char* name, size_t command_buffer_size = 100);
        Thread(const String& name, size_t command_buffer_size = 100);

        const String& name() const;
        Thread& name(const String& thread_name);
        Thread& name(const char* thread_name);

        bool has_unfinished_tasks() const;
        bool unfinished_tasks_count() const;
        bool is_thread_sleep() const;
        bool is_busy() const;
        bool is_shutting_down() const;

        Thread& wait_all();
        Thread& remove_all_tasks();

        static Thread* this_thread();

        Thread& push_task(Task task);

        ~Thread();
    };
}// namespace Engine
