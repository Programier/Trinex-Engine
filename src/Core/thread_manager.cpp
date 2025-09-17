#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/thread.hpp>
#include <Core/thread_manager.hpp>
#include <Engine/settings.hpp>

namespace Engine
{
	static ThreadManager* m_manager = nullptr;

	class WorkerThread : public Thread
	{
		std::thread m_thread;

	public:
		using Thread::register_thread;
		using Thread::register_thread_name;
		using Thread::unregister_thread;

		template<typename... Args>
		WorkerThread(Args... args) : Thread(NoThread(), 1024 * 16), m_thread(args..., this)
		{}

		~WorkerThread()
		{
			if (m_thread.joinable())
			{
				m_thread.join();
			}
		}
	};

	struct CompletedTask : public TaskInterface {
		uint32_t m_size;
		static constexpr uint64_t complete_mask = 0xFFFFFFFF00000000ull;
		static constexpr uint64_t size_mask     = 0x00000000FFFFFFFFull;

		CompletedTask(uint32_t size) : m_size(size) {}
		void execute() override {}
		size_t size() const override { return complete_mask | m_size; }
	};

	ThreadManager* ThreadManager::instance()
	{
		if (m_manager == nullptr)
		{
			m_manager = new ThreadManager();
		}
		return m_manager;
	}

	ThreadManager::ThreadManager()
	{
		PostDestroyController().push(destroy_manager);

		m_read_pointer        = m_buffer;
		m_thread_read_pointer = m_buffer;
		m_write_pointer       = m_buffer;


		uint_t threads_count = 10;
		m_threads.reserve(threads_count);

		for (size_t i = 0; i < threads_count; ++i)
		{
			WorkerThread* thread = trx_new WorkerThread(&ThreadManager::thread_loop, this);
			m_threads.push_back(thread);
		}
	}

	ThreadManager::~ThreadManager()
	{
		m_running   = false;
		m_has_tasks = true;
		m_has_tasks.notify_all();

		for (auto& thread : m_threads)
		{
			trx_delete thread;
		}
	}

	void ThreadManager::thread_loop(WorkerThread* thread)
	{
		thread->register_thread();

		while (m_running)
		{
			while (m_running && m_read_pointer == m_write_pointer)
			{
				m_has_tasks = false;
				m_has_tasks.wait(false);
			}

			if (!m_running)
				return;

			thread->execute_commands();

			TaskInterface* task = begin_execute();

			if (task == nullptr)
				continue;

			task->execute();
			end_execute(task);
		}

		thread->unregister_thread();
	}

	TaskInterface* ThreadManager::begin_execute()
	{
		ScopeLock lock(m_task_exec_section);

		byte* rp = m_thread_read_pointer.load();
		byte* wp = m_write_pointer;

		if (rp == wp)
		{
			return nullptr;
		}

		TaskInterface* task = reinterpret_cast<TaskInterface*>(rp);
		rp                  = align_memory(rp + task->size(), m_align);

		if (rp >= m_buffer + m_buffer_size)
			rp = m_buffer;

		m_thread_read_pointer = rp;
		return task;
	}

	void ThreadManager::end_execute(TaskInterface* task)
	{
		ScopeLock lock(m_task_exec_section);

		byte* rp              = m_read_pointer.load();
		byte* const thread_rp = m_thread_read_pointer.load();

		if (rp == reinterpret_cast<byte*>(task))
		{
			do
			{
				rp = align_memory(rp + (task->size() & CompletedTask::size_mask), m_align);
				task->~TaskInterface();

				if (rp >= m_buffer + m_buffer_size)
					rp = m_buffer;

				task = reinterpret_cast<TaskInterface*>(rp);
			} while (rp != thread_rp && (task->size() & CompletedTask::complete_mask) == CompletedTask::complete_mask);

			m_read_pointer = rp;
		}
		else
		{
			size_t size = task->size();
			task->~TaskInterface();
			new (task) CompletedTask(size);
		}
	}

	void ThreadManager::destroy_manager()
	{
		delete m_manager;
		m_manager = nullptr;
	}
}// namespace Engine
