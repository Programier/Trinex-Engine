#pragma once
#include <Core/engine_types.hpp>
#include <Core/exception.hpp>
#include <Core/executable_object.hpp>
#include <Core/ring_buffer.hpp>
#include <condition_variable>
#include <thread>

namespace Engine
{
    static constexpr inline size_t default_thread_command_buffer_size = 1024 * 1024 * 1;// 1 MB


    class ENGINE_EXPORT SkipThreadCommand : public ExecutableObject
    {
    private:
        size_t _M_skip_bytes;

    public:
        SkipThreadCommand(size_t bytes);
        int_t execute() override;
    };


    class ENGINE_EXPORT ThreadBase
    {
    protected:
        Identifier _M_id;
        String _M_name;
        std::recursive_mutex _M_edit_mutex;
        std::thread::native_handle_type _M_native_handle;

        ThreadBase();

        void update_id();
        void update_name();


    public:
        static void sleep_for(float seconds);
        static ThreadBase* this_thread();

        const String& name() const;
        ThreadBase& name(const String& thread_name);
        virtual bool is_destroyable();
        Identifier id();
        virtual ~ThreadBase();
    };


    class ENGINE_EXPORT Thread : public ThreadBase
    {
    private:
        std::mutex _M_exec_mutex;
        std::condition_variable _M_exec_cv;

        std::mutex _M_wait_mutex;
        std::condition_variable _M_wait_cv;

        RingBuffer _M_command_buffer;
        std::thread* _M_thread;

        std::atomic<bool> _M_is_shuting_down;
        std::atomic<bool> _M_is_thread_busy;

        struct ENGINE_EXPORT NewTaskEvent : public ExecutableObject
        {
        public:
            Thread* _M_thread;
            int_t execute() override;
        };

        NewTaskEvent _M_event;


        static void thread_loop(Thread* self);


    public:
        Thread(size_t command_buffer_size = default_thread_command_buffer_size);
        Thread(const String& name, size_t command_buffer_size = default_thread_command_buffer_size);

        bool is_thread_sleep() const;
        bool is_busy() const;
        Thread& wait_all();
        bool is_destroyable() override;
        const RingBuffer& command_buffer() const;

        template<typename CommandType, typename Function, typename... Args>
        FORCE_INLINE Thread& insert_new_task_with_initializer(Function&& initializer, Args&&... args)
        {
            RingBuffer::AllocationContext context(_M_command_buffer, sizeof(CommandType));
            if (context.size() < sizeof(CommandType))
            {
                new (context.data()) SkipThreadCommand(context.size());
                context.submit();
                RingBuffer::AllocationContext new_context(_M_command_buffer, sizeof(CommandType));
                if (new_context.size() < sizeof(CommandType))
                {
                    throw EngineException("Cannot push new task to thread");
                }

                CommandType* command = new (new_context.data()) CommandType(std::forward<Args>(args)...);
                initializer(command);
            }
            else
            {
                CommandType* command = new (context.data()) CommandType(std::forward<Args>(args)...);
                initializer(command);
            }
            return *this;
        }

        template<typename CommandType, typename... Args>
        FORCE_INLINE Thread& insert_new_task(Args&&... args)
        {
            RingBuffer::AllocationContext context(_M_command_buffer, sizeof(CommandType));
            if (context.size() < sizeof(CommandType))
            {
                new (context.data()) SkipThreadCommand(context.size());
                context.submit();
                RingBuffer::AllocationContext new_context(_M_command_buffer, sizeof(CommandType));
                if (new_context.size() < sizeof(CommandType))
                {
                    throw EngineException("Cannot push new task to thread");
                }

                new (new_context.data()) CommandType(std::forward<Args>(args)...);
            }
            else
            {
                new (context.data()) CommandType(std::forward<Args>(args)...);
            }
            return *this;
        }

        virtual ~Thread();
    };


}// namespace Engine
