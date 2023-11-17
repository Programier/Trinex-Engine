#include <Core/thread.hpp>
#include <chrono>
#include <thread>


namespace Engine
{
    static thread_local ThreadBase* this_thread_instance = nullptr;

#if PLATFORM_WINDOWS
    static constexpr inline std::size_t max_thread_name_length = 32;
#else
    static constexpr inline std::size_t max_thread_name_length = 16;
#endif

    SkipThreadCommand::SkipThreadCommand(size_t bytes) : _M_skip_bytes(bytes)
    {}

    int_t SkipThreadCommand::execute()
    {
        return static_cast<int>(_M_skip_bytes);
    }

    ThreadBase::ThreadBase()
    {
        _M_native_handle = pthread_self();

        update_id();
        update_name();
    }

    void ThreadBase::update_id()
    {
        std::unique_lock lock(_M_edit_mutex);
        _M_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
    }

    void ThreadBase::update_name()
    {
        std::unique_lock lock(_M_edit_mutex);
        char thread_name[max_thread_name_length];
        if (pthread_getname_np(_M_native_handle, thread_name, sizeof(thread_name)) == 0)
        {
            _M_name = thread_name;
        }
    }


    void ThreadBase::sleep_for(float seconds)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<size_t>(seconds * 1000000.f)));
    }

    ThreadBase* ThreadBase::this_thread()
    {
        if (this_thread_instance == nullptr)
        {
            static thread_local ThreadBase _this_thread;
            this_thread_instance = &_this_thread;
        }

        return this_thread_instance;
    }

    const String& ThreadBase::name() const
    {
        return _M_name;
    }

    ThreadBase& ThreadBase::name(const String& thread_name)
    {
        std::unique_lock lock(_M_edit_mutex);
        _M_name =
                thread_name.substr(0, std::min<size_t>(max_thread_name_length, static_cast<int>(thread_name.length())));
        pthread_setname_np(_M_native_handle, _M_name.c_str());
        return *this;
    }

    bool ThreadBase::is_destroyable()
    {
        return false;
    }

    Identifier ThreadBase::id()
    {
        return _M_id;
    }

    ThreadBase::~ThreadBase()
    {}

    void Thread::thread_loop(Thread* self)
    {
        this_thread_instance = self;
        self->update_id();
        while (!self->_M_is_shuting_down.load())
        {
            {
                std::unique_lock lock(self->_M_exec_mutex);

                self->_M_is_thread_busy.store(false);
                self->_M_wait_cv.notify_all();

                self->_M_exec_cv.wait(lock, [self]() -> bool {
                    return self->_M_command_buffer.unreaded_buffer_size() != 0 || self->_M_is_shuting_down;
                });

                if (self->_M_is_shuting_down.load())
                    return;

                self->_M_is_thread_busy.store(true);
            }

            ExecutableObject* object = nullptr;

            while ((object = reinterpret_cast<ExecutableObject*>(self->_M_command_buffer.reading_pointer())))
            {
                int_t size = object->execute();
                std::destroy_at(object);
                self->_M_command_buffer.finish_read(size);
            }
        }

        self->_M_wait_cv.notify_all();
    }


    Thread::Thread(size_t size)
    {
        _M_is_shuting_down.store(false);
        _M_command_buffer.init(size);
        _M_thread        = new std::thread(&Thread::thread_loop, this);
        _M_native_handle = _M_thread->native_handle();

        update_name();
    }

    Thread::Thread(const String& thread_name, size_t size) : Thread(size)
    {
        name(thread_name);
    }

    bool Thread::is_thread_sleep() const
    {
        return !is_busy();
    }

    bool Thread::is_busy() const
    {
        return _M_is_thread_busy.load();
    }

    Thread& Thread::wait_all()
    {
        if (Thread::this_thread() != this)
        {
            std::unique_lock lock(_M_wait_mutex);
            _M_exec_cv.notify_all();

            _M_wait_cv.wait(lock, [this]() -> bool {
                return _M_is_shuting_down || (is_thread_sleep() && _M_command_buffer.unreaded_buffer_size() == 0);
            });
        }
        return *this;
    }

    bool Thread::is_destroyable()
    {
        return true;
    }

    const RingBuffer& Thread::command_buffer() const
    {
        return _M_command_buffer;
    }

    Thread::~Thread()
    {
        {
            std::unique_lock lock(_M_exec_mutex);
            _M_is_shuting_down.store(true);
        }

        _M_exec_cv.notify_all();
        _M_thread->join();
        delete _M_thread;
    }
}// namespace Engine
