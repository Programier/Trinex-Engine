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
		size_t m_skip_bytes;

	public:
		SkipThreadCommand(size_t bytes);
		int_t execute() override;
	};


	class ENGINE_EXPORT ThreadBase
	{
	protected:
		Identifier m_id;
		String m_name;
		std::recursive_mutex m_edit_mutex;
		std::thread::native_handle_type m_native_handle;

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
		template<typename Func>
		struct FunctionCaller : public ExecutableObject {
			std::decay_t<Func> m_func;

		public:
			FunctionCaller(Func&& func) : m_func(std::forward<Func>(func))
			{}

			int_t execute() override
			{
				m_func();
				return sizeof(*this);
			}
		};


		std::mutex m_exec_mutex;
		std::condition_variable m_exec_cv;

		std::mutex m_wait_mutex;
		std::condition_variable m_wait_cv;

		RingBuffer m_command_buffer;
		std::thread* m_thread;

		std::atomic<bool> m_is_shuting_down;
		std::atomic<bool> m_is_thread_busy;

		struct ENGINE_EXPORT NewTaskEvent : public ExecutableObject {
		public:
			Thread* m_thread;
			int_t execute() override;
		};

		NewTaskEvent m_event;

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
			RingBuffer::AllocationContext context(m_command_buffer, sizeof(CommandType));
			if (context.size() < sizeof(CommandType))
			{
				new (context.data()) SkipThreadCommand(context.size());
				context.submit();
				RingBuffer::AllocationContext new_context(m_command_buffer, sizeof(CommandType));
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
			RingBuffer::AllocationContext context(m_command_buffer, sizeof(CommandType));
			if (context.size() < sizeof(CommandType))
			{
				new (context.data()) SkipThreadCommand(context.size());
				context.submit();
				RingBuffer::AllocationContext new_context(m_command_buffer, sizeof(CommandType));
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

		template<typename Function>
		FORCE_INLINE Thread& call_function(Function&& function)
		{
			return insert_new_task<FunctionCaller<Function>>(std::forward<Function>(function));
		}

		template<typename Function, typename... Args>
		FORCE_INLINE Thread& call_function(Function&& function, Args&&... args)
		{
			auto new_function = std::bind(std::forward<Function>(function), std::forward<Args>(args)...);
			return insert_new_task<FunctionCaller<decltype(new_function)>>(std::move(new_function));
		}

		virtual ~Thread();
	};


}// namespace Engine
