#include <Core/etl/ring_buffer.hpp>
#include <Core/executable_object.hpp>
#include <Core/logger.hpp>
#include <Platform/thread.hpp>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <thread>

namespace Engine
{
    static thread_local class PlatformThreadBase* this_thread_instance = nullptr;

#ifndef MAX_THREAD_NAME_LENGTH
#define MAX_THREAD_NAME_LENGTH 16
#endif

    class PlatformThreadBase : public Thread
    {
    protected:
        Identifier _M_id = 0;
        String _M_name;
        std::thread* _M_thread = nullptr;

        mutable std::mutex _M_edit_mutex;


    public:
        PlatformThreadBase()
        {
            char thread_name[MAX_THREAD_NAME_LENGTH];// Adjust the buffer size as needed
            if (pthread_getname_np(native_handle(), thread_name, sizeof(thread_name)) == 0)
            {
                _M_name = thread_name;
            }
            update_thread_id();
        }

        void update_thread_id()
        {
            _M_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
        }

        std::thread::native_handle_type native_handle() const
        {
            if (_M_thread)
            {
                return _M_thread->native_handle();
            }

            return pthread_self();
        }

        PlatformThreadBase& update_name(const String& new_name)
        {
            std::unique_lock lock(_M_edit_mutex);
            _M_name = new_name.substr(0, std::min(MAX_THREAD_NAME_LENGTH, static_cast<int>(new_name.length())));
            pthread_setname_np(native_handle(), _M_name.c_str());
            return *this;
        }

        const String& name() const override
        {
            return _M_name;
        }

        Thread& name(const String& thread_name) override
        {
            return update_name(thread_name.c_str());
        }

        bool has_unfinished_tasks() const override
        {
            return false;
        }

        bool is_thread_sleep() const override
        {
            return false;
        }

        bool is_busy() const override
        {
            return true;
        }

        bool is_shutting_down() const override
        {
            return false;
        }

        Thread& wait_all() override
        {
            return *this;
        }

        Thread& push_task(Task task) override
        {
            error_log("Thread", "Cannot push task to external thread!");
            return *this;
        }

        Identifier id() override
        {
            return _M_id;
        }

        bool is_destroyable() override
        {
            return false;
        }
    };


    class PlatformThread : public PlatformThreadBase
    {

        RingBuffer<Task> _M_tasks;

        mutable std::mutex _M_exec_mutex;
        mutable std::mutex _M_wait_mutex;

        std::condition_variable _M_exec_cv;
        std::condition_variable _M_wait_cv;

        bool _M_is_shutting_down = false;
        bool _M_is_thread_sleep  = false;


        bool is_shutting_down_protected() const
        {
            return _M_is_shutting_down;
        }

        bool has_unfinished_tasks_protected() const
        {
            return !_M_tasks.empty();
        }

    public:
        static void thread_loop(PlatformThread* self)
        {
            this_thread_instance = self;
            self->update_thread_id();

            while (!self->_M_is_shutting_down)
            {
                Task task;

                {
                    std::unique_lock lock(self->_M_exec_mutex);

                    self->_M_is_thread_sleep = true;
                    self->_M_wait_cv.notify_all();

                    self->_M_exec_cv.wait(lock, [self]() {
                        return self->is_shutting_down_protected() || self->has_unfinished_tasks_protected();
                    });
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

        Task next_task()
        {
            Task task = _M_tasks.front();
            _M_tasks.pop();
            return task;
        }


        PlatformThread(const String& thread_name, size_t command_buffer_size) : _M_tasks(command_buffer_size)
        {
            _M_thread = new std::thread(thread_loop, this);
            update_name(thread_name.c_str());
        }

        bool has_unfinished_tasks() const override
        {
            std::unique_lock lock(_M_exec_mutex);
            return has_unfinished_tasks_protected();
        }

        bool is_thread_sleep() const override
        {
            std::unique_lock lock(_M_exec_mutex);
            return _M_is_thread_sleep;
        }

        bool is_busy() const override
        {
            return !is_thread_sleep();
        }

        bool is_shutting_down() const override
        {
            std::unique_lock lock(_M_exec_mutex);
            return is_shutting_down_protected();
        }

        Thread& wait_all() override
        {
            if (Thread::this_thread() != this)
            {
                std::unique_lock lock(_M_wait_mutex);
                _M_wait_cv.wait(lock, [this]() { return !has_unfinished_tasks() && is_thread_sleep(); });
            }
            return *this;
        }

        Thread& push_task(Task task) override
        {
            std::unique_lock lock1(_M_exec_mutex);
            if (_M_tasks.max_size() == _M_tasks.size())
            {
                if (this_thread() != this_thread_instance)
                {
                    std::unique_lock lock(_M_wait_mutex);
                    _M_wait_cv.wait(lock, [this]() { return _M_tasks.max_size() == _M_tasks.size(); });
                    _M_tasks.push(task);
                }
                else
                {
                    throw EngineException("Cannot push new task!");
                }
            }
            else
            {
                _M_tasks.push(task);
            }
            _M_exec_cv.notify_all();
            return *this;
        }

        ~PlatformThread()
        {
            {
                std::unique_lock lock(_M_exec_mutex);
                _M_is_shutting_down = true;
            }

            _M_exec_cv.notify_all();
            _M_thread->join();
            delete _M_thread;
        }

        bool is_destroyable() override
        {
            return true;
        }
    };


    Thread* Thread::this_thread()
    {
        if (this_thread_instance == nullptr)
        {
            static thread_local PlatformThreadBase thread;
            this_thread_instance = &thread;
        }
        return this_thread_instance;
    }

    Thread* Thread::new_thread(const String& name, size_t command_buffer_size)
    {
        return new PlatformThread(name, command_buffer_size);
    }
}// namespace Engine
