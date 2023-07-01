#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Core/thread.hpp>
#include <SDL_thread.h>
#include <cerrno>
#include <cstring>
#include <pthread.h>

namespace Engine
{
    static thread_local Thread* _M_instance = nullptr;


    Thread::Task::Task() = default;

    Thread::Task::Task(const Function<void()>& func, bool is_unique_command)
        : func(func), is_unique_command(is_unique_command)
    {}

    Thread::Task::Task(Function<void()>&& func, bool is_unique_command)
        : func(std::move(func)), is_unique_command(is_unique_command)
    {}


    Thread::Task::Task(const Task&) = default;
    Thread::Task::Task(Task&&)      = default;

    Thread::Task& Thread::Task::operator=(const Task&) = default;
    Thread::Task& Thread::Task::operator=(Task&&)      = default;

    Thread::Thread()
    {
        _M_current = _M_tasks.begin();

        _M_thread_loop = [this]() {
            _M_instance = this;

            while (!_M_is_shutting_down)
            {
                Function<void()> task;

                {
                    std::unique_lock lock(_M_mutex);

                    _M_is_thread_sleep = true;
                    _M_cv_for_wait.notify_all();

                    _M_cv.wait(lock, [this]() { return _M_is_shutting_down || has_unfinished_tasks(); });
                    _M_is_thread_sleep = false;

                    if (_M_is_shutting_down)
                    {
                        return;
                    }

                    task = next_task();
                }

                task();
            }
        };

        _M_thread = new std::thread(_M_thread_loop);

        char thread_name[256];
        if (pthread_getname_np(_M_thread->native_handle(), thread_name, sizeof(thread_name)) == 0)
        {
            _M_name = thread_name;
        }
    }


    Thread::Thread(const char* thread_name) : Thread()
    {
        name(thread_name);
    }

    Thread::Thread(const String& thread_name)
    {
        name(thread_name);
    }

    const String& Thread::name() const
    {
        return _M_name;
    }

    bool Thread::platform_thread_change_name(const char* _name)
    {
        const char* name      = nullptr;
        bool need_delete_name = false;
        bool result           = false;

        if (std::strlen(_name) > Thread::max_thread_name_length)
        {
            name = new char[Thread::max_thread_name_length + 1];
            std::memcpy(const_cast<char*>(name), _name, Thread::max_thread_name_length);
            const_cast<char*>(name)[Thread::max_thread_name_length] = '\0';
            need_delete_name                                        = true;
        }
        else
        {
            name = _name;
        }

        auto handle = _M_thread->native_handle();
        if (pthread_setname_np(handle, name) == 0)
        {
            result  = true;
            _M_name = name;
        }

        if (need_delete_name)
        {
            delete name;
        }

        return result;
    }

    Thread& Thread::name(const String& thread_name)
    {
        return name(thread_name.c_str());
    }

    Thread& Thread::name(const char* thread_name)
    {
        std::unique_lock lock(_M_mutex);
        if (!platform_thread_change_name(thread_name))
        {
            logger->error("Thread: Failed to change thread name!\n\tReason: %s\n", strerror(errno));
        }
        return *this;
    }

    Function<void()> Thread::next_task()
    {
        Task& task            = *_M_current;
        Function<void()> func = task.is_unique_command ? std::move(task.func) : task.func;

        if (task.is_unique_command)
        {
            _M_tasks.erase(_M_current++);
        }
        else
        {
            ++_M_current;
        }

        if (_M_auto_restart && _M_current == _M_tasks.end())
        {
            _M_current = _M_tasks.begin();
        }
        return func;
    }

    bool Thread::has_unfinished_tasks() const
    {
        return _M_current != _M_tasks.end();
    }

    bool Thread::unfinished_tasks_count() const
    {
        return _M_tasks.size();
    }

    const List<Thread::Task>& Thread::tasks() const
    {
        return _M_tasks;
    }

    bool Thread::is_thread_sleep() const
    {
        return _M_is_thread_sleep;
    }

    bool Thread::auto_restart() const
    {
        return _M_auto_restart;
    }

    Thread& Thread::auto_restart(bool flag)
    {
        std::unique_lock lock(_M_mutex);
        _M_auto_restart = flag;
        return *this;
    }


    Thread& Thread::restart_tasks()
    {
        std::unique_lock lock(_M_mutex);

        if (!has_unfinished_tasks())
        {
            _M_current = _M_tasks.begin();
            _M_cv.notify_all();
        }
        return *this;
    }

    Thread& Thread::wait_all()
    {
        if (Thread::this_thread() != this)
        {
            std::unique_lock lock(_M_mutex_for_wait);
            _M_cv_for_wait.wait(lock, [this]() { return !has_unfinished_tasks(); });
        }
        return *this;
    }

    Thread& Thread::remove_all_tasks()
    {
        std::unique_lock lock(_M_mutex);
        _M_tasks.clear();
        _M_current = _M_tasks.begin();
        return *this;
    }

    Thread* Thread::this_thread()
    {
        return _M_instance;
    }

    Thread& Thread::on_task_pushed()
    {
        if (_M_tasks.size() == 1)
        {
            _M_current = _M_tasks.begin();
        }

        _M_cv.notify_all();
        return *this;
    }

    Thread& Thread::push_task(const Task& task)
    {
        std::unique_lock lock(_M_mutex);
        _M_tasks.push_back(task);
        return on_task_pushed();
    }

    Thread& Thread::push_task(Task&& task)
    {
        std::unique_lock lock(_M_mutex);
        _M_tasks.push_back(std::move(task));
        return on_task_pushed();
    }

    Thread::~Thread()
    {
        {
            std::unique_lock lock(_M_mutex);
            _M_is_shutting_down = true;
        }

        _M_cv.notify_all();

        _M_thread->join();
        delete _M_thread;
    }
}// namespace Engine
