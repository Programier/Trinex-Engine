#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Core/thread.hpp>
#include <cerrno>
#include <cstring>
#include <pthread.h>

namespace Engine
{
    static thread_local Thread* _M_instance = nullptr;


    void Thread::thread_loop(Thread* self)
    {
        _M_instance = self;

        while (!self->_M_is_shutting_down)
        {
            Task task;

            {
                std::unique_lock lock(self->_M_mutex);

                self->_M_is_thread_sleep = true;
                self->_M_cv_for_wait.notify_all();

                self->_M_cv.wait(lock, [self]() { return self->is_shutting_down() || self->has_unfinished_tasks(); });
                self->_M_is_thread_sleep = false;

                if (self->_M_is_shutting_down)
                {
                    return;
                }

                task = self->next_task();
            }

            task->execute();
        }
    }

    Thread::Thread(size_t command_buffer_size) : _M_tasks(command_buffer_size)
    {
        _M_thread = new std::thread(thread_loop, this);

        char thread_name[256];
        if (pthread_getname_np(_M_thread->native_handle(), thread_name, sizeof(thread_name)) == 0)
        {
            _M_name = thread_name;
        }
    }


    Thread::Thread(const char* thread_name, size_t command_buffer_size) : Thread(command_buffer_size)
    {
        name(thread_name);
    }

    Thread::Thread(const String& thread_name, size_t command_buffer_size) : Thread(command_buffer_size)
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
            result = true;
            std::unique_lock lock(_M_mutex2);
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
        if (!platform_thread_change_name(thread_name))
        {
            error_log("Thread: Failed to change thread name!\n\tReason: %s\n", strerror(errno));
        }
        return *this;
    }

    Thread::Task Thread::next_task()
    {
        Task task = _M_tasks.front();
        _M_tasks.pop();
        return task;
    }

    bool Thread::has_unfinished_tasks() const
    {
        if (_M_instance != this)
        {
            std::unique_lock lock(_M_mutex);
            std::unique_lock lock2(_M_mutex2);
            return !_M_tasks.empty();
        }
        else
        {
            std::unique_lock lock2(_M_mutex2);
            return !_M_tasks.empty();
        }
    }

    bool Thread::unfinished_tasks_count() const
    {
        std::unique_lock lock(_M_mutex);
        std::unique_lock lock2(_M_mutex2);
        return _M_tasks.size();
    }


    bool Thread::is_thread_sleep() const
    {
        std::unique_lock lock(_M_mutex);
        std::unique_lock lock2(_M_mutex2);
        return _M_is_thread_sleep;
    }

    bool Thread::is_busy() const
    {
        return !is_thread_sleep();
    }

    bool Thread::is_shutting_down() const
    {
        if (_M_instance == this)
        {
            std::unique_lock lock2(_M_mutex2);
            return _M_is_shutting_down;
        }
        else
        {
            std::unique_lock lock(_M_mutex);
            std::unique_lock lock2(_M_mutex2);
            return _M_is_shutting_down;
        }
    }


    Thread& Thread::wait_all()
    {
        if (Thread::this_thread() != this)
        {
            std::unique_lock lock(_M_mutex_for_wait);
            _M_cv_for_wait.wait(lock, [this]() { return !has_unfinished_tasks() || is_busy(); });
        }
        return *this;
    }

    Thread& Thread::remove_all_tasks()
    {
        std::unique_lock lock(_M_mutex);
        std::unique_lock lock2(_M_mutex2);
        _M_tasks.clear();
        return *this;
    }

    Thread* Thread::this_thread()
    {
        return _M_instance;
    }

    Thread& Thread::on_task_pushed()
    {
        _M_cv.notify_all();
        return *this;
    }

    Thread& Thread::push_task(Task task)
    {
        std::unique_lock lock(_M_mutex);
        std::unique_lock lock2(_M_mutex2);
        if (_M_tasks.max_size() == _M_tasks.size() && _M_instance != this)
        {
            std::unique_lock lock(_M_mutex_for_wait);
            _M_cv_for_wait.wait(lock);
            _M_tasks.push(task);
        }
        else
        {
            _M_tasks.push(task);
        }
        return on_task_pushed();
    }

    Thread::~Thread()
    {
        {
            std::unique_lock lock(_M_mutex);
            std::unique_lock lock2(_M_mutex2);
            _M_is_shutting_down = true;
        }

        _M_cv.notify_all();

        _M_thread->join();
        delete _M_thread;
    }
}// namespace Engine
