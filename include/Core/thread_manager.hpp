#pragma once
#include <Core/etl/critical_section.hpp>
#include <Core/etl/function.hpp>
#include <Core/etl/vector.hpp>
#include <Core/memory.hpp>
#include <Core/task.hpp>
#include <thread>

namespace Engine
{
	class ENGINE_EXPORT ThreadManager final
	{
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

		static constexpr inline size_t m_buffer_size = 1024 * 1024 * 1;
		static constexpr inline size_t m_align       = 16;

		alignas(m_align) byte m_buffer[m_buffer_size];
		Vector<class WorkerThread*> m_threads;

		Atomic<byte*> m_thread_read_pointer;
		Atomic<byte*> m_read_pointer;
		Atomic<byte*> m_write_pointer;

		Atomic<bool> m_running = true;
		Atomic<bool> m_has_tasks;

		CriticalSection m_task_exec_section;
		CriticalSection m_push_task_section;

	protected:
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
			size += m_align;
			while (free_size() < size)
			{
				std::this_thread::yield();
			}
		}


		ThreadManager();
		~ThreadManager();

		void thread_loop(class WorkerThread* thread);
		TaskInterface* begin_execute();
		void end_execute(TaskInterface* interface);

	public:
		static void destroy_manager();
		static ThreadManager* instance();

		template<typename CommandType, typename... Args>
		inline ThreadManager& create_task(Args&&... args)
		{
			m_push_task_section.lock();

			size_t task_size = align_up(sizeof(CommandType), m_align);
			byte* wp         = m_write_pointer;

			if (wp + task_size > m_buffer + m_buffer_size)
			{
				wait_for_size(sizeof(SkipBytes));
				new (wp) SkipBytes((m_buffer + m_buffer_size) - wp);
				wp = m_buffer;
			}

			wait_for_size(task_size);

			new (wp) CommandType(std::forward<Args>(args)...);
			wp += task_size;

			if (wp >= m_buffer + m_buffer_size)
			{
				wp = m_buffer;
			}

			m_write_pointer = wp;
			m_push_task_section.unlock();

			m_has_tasks = true;
			m_has_tasks.notify_one();
			return *this;
		}

		template<typename Function>
		FORCE_INLINE ThreadManager& call(Function&& function)
		{
			return create_task<FunctionCaller<Function>>(std::forward<Function>(function));
		}

		template<typename Function, typename... Args>
		FORCE_INLINE ThreadManager& call(Function&& function, Args&&... args)
		{
			auto new_function = std::bind(std::forward<Function>(function), std::forward<Args>(args)...);
			return create_task<FunctionCaller<decltype(new_function)>>(std::move(new_function));
		}
	};
}// namespace Engine
