#pragma once

#include <Core/engine_types.hpp>
#include <condition_variable>
#include <future>


namespace Engine
{
    class ENGINE_EXPORT Thread final
    {
    public:
        struct Task {
            Function<void()> func;
            bool is_unique_command = false;

            Task();
            Task(const Function<void()>& func, bool is_unique_command = false);
            Task(Function<void()>&& func, bool is_unique_command = false);
            Task(const Task&);
            Task(Task&&);

            Task& operator=(const Task&);
            Task& operator=(Task&&);
        };

#if PLATFORM_WINDOWS
        static constexpr size_t max_thread_name_length = 31;
#else
        static constexpr size_t max_thread_name_length = 15;
#endif

    private:
        std::condition_variable _M_cv;
        std::condition_variable _M_cv_for_wait;
        std::mutex _M_mutex;
        std::mutex _M_mutex_for_wait;

        Function<void()> _M_thread_loop;
        String _M_name;

        List<Task> _M_tasks;
        List<Task>::iterator _M_current;

        std::thread* _M_thread = nullptr;

        bool _M_is_shutting_down      = false;
        bool _M_is_thread_sleep       = false;
        bool _M_auto_restart          = false;

        bool platform_thread_change_name(const char* name);
        Thread& on_task_pushed();

        Function<void()> next_task();

    public:
        Thread();
        Thread(const char* name);
        Thread(const String& name);

        const String& name() const;
        Thread& name(const String& thread_name);
        Thread& name(const char* thread_name);

        bool has_unfinished_tasks() const;
        bool unfinished_tasks_count() const;
        const List<Task>& tasks() const;
        bool is_thread_sleep() const;
        bool auto_restart() const;

        Thread& auto_restart(bool flag);
        Thread& restart_tasks();
        Thread& wait_all();

        static Thread* this_thread();

        Thread& push_task(const Task& task);
        Thread& push_task(Task&& task);

        template<typename Fn, typename... Args>
        Thread& push_task(Fn&& function, Args&&... args)
        {
            return push_task(Task(std::bind(std::forward<Fn>(function), std::forward<Args>(args)...), false));
        }

        template<typename Fn, typename... Args>
        Thread& push_unique_task(Fn&& function, Args&&... args)
        {
            return push_task(Task(std::bind(std::forward<Fn>(function), std::forward<Args>(args)...), true));
        }

        ~Thread();
    };
}// namespace Engine
