#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/atomic.hpp>
#include <Core/etl/critical_section.hpp>
#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <Core/threading/task.hpp>
#include <functional>
#include <thread>

namespace Engine
{
	class ENGINE_EXPORT Thread
	{
	public:
		static constexpr inline size_t command_alignment = 16;

	protected:
		Thread& register_thread();
		Thread& unregister_thread();

	protected:
		template<typename Func>
		struct FunctionCaller : public Task<FunctionCaller<Func>> {
			std::decay_t<Func> m_func;

		public:
			FunctionCaller(Func&& func) : m_func(std::forward<Func>(func)) {}

			void execute() override { m_func(); }
		};

		struct SkipBytes : public Task<SkipBytes> {
			int_t m_bytes;
			SkipBytes(int_t bytes) : m_bytes(bytes) {}

			size_t size() const override { return m_bytes; }

			void execute() override {}
		};

		struct NoThread {
		};

		byte* const m_buffer;
		const size_t m_buffer_size;

		std::thread* m_thread = nullptr;

		Atomic<byte*> m_read_pointer;
		Atomic<byte*> m_write_pointer;

		Atomic<bool> m_running = true;

		std::atomic_flag m_exec_flag = ATOMIC_FLAG_INIT;
		CriticalSection m_push_section;

	protected:
		void thread_loop();

		inline size_t free_size() const
		{
			byte* rp = m_read_pointer;
			byte* wp = m_write_pointer;

			if (rp <= wp)
			{
				return m_buffer_size - (wp - rp);
			}
			else
			{
				return rp - wp;
			}
		}

		FORCE_INLINE void wait_for_size(size_t size)
		{
			size += command_alignment;
			while (free_size() < size)
			{
				std::this_thread::yield();
			}
		}

		Thread(NoThread, size_t command_buffer_size = 1024 * 1024 * 1);

	public:
		Thread(size_t command_buffer_size = 1024 * 1024 * 1);
		Thread& execute_commands();
		Thread& execute_commands(uint_t commands_limit);
		Thread& wait();

		static void static_sleep_for(float seconds);
		static Thread* static_self();
		static void static_yield();

		template<typename CommandType, typename... Args>
		inline Thread& create_task(Args&&... args)
		{
			m_push_section.lock();

			size_t task_size = align_up(sizeof(CommandType), command_alignment);
			byte* wp         = m_write_pointer;

			if (wp + task_size > m_buffer + m_buffer_size)
			{
				wait_for_size(sizeof(SkipBytes));
				new (wp) SkipBytes((m_buffer + m_buffer_size) - wp);
				wp              = m_buffer;
				m_write_pointer = wp;
			}

			wait_for_size(task_size);

			new (wp) CommandType(std::forward<Args>(args)...);
			wp += task_size;

			if (wp >= m_buffer + m_buffer_size)
			{
				wp = m_buffer;
			}

			m_write_pointer = wp;
			m_exec_flag.test_and_set();
			m_exec_flag.notify_all();

			m_push_section.unlock();
			return *this;
		}

		template<typename Function>
		FORCE_INLINE Thread& call(Function&& function)
		{
			return create_task<FunctionCaller<Function>>(std::forward<Function>(function));
		}

		template<typename Function, typename... Args>
		FORCE_INLINE Thread& call(Function&& function, Args&&... args)
		{
			auto new_function = std::bind(std::forward<Function>(function), std::forward<Args>(args)...);
			return create_task<FunctionCaller<decltype(new_function)>>(std::move(new_function));
		}

		virtual ~Thread();
	};
}// namespace Engine
