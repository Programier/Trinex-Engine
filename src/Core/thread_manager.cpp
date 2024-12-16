#include <Core/engine_loading_controllers.hpp>
#include <Core/thread_manager.hpp>

namespace Engine
{
	static ThreadManager* m_manager = nullptr;

	ThreadManager* ThreadManager::instance()
	{
		if (m_manager == nullptr)
		{
			m_manager = new ThreadManager();
		}
		return m_manager;
	}

	ThreadManager& ThreadManager::resize(size_t threads_count)
	{
		threads_count = glm::clamp<size_t>(1, std::thread::hardware_concurrency(), threads_count);

		for (size_t i = 0; i < threads_count; ++i)
		{
			m_threads.emplace_back(&ThreadManager::thread_loop, this);
		}
		return *this;
	}

	void ThreadManager::thread_loop()
	{
		while (m_running)
		{
			while (m_running && m_read_pointer == m_write_pointer)
			{
				m_has_tasks = false;
				m_has_tasks.wait(false);
			}

			if (!m_running)
				return;


			// Load and unregister task from thread manager
			m_thread_section.lock();
			byte* rp = m_thread_read_pointer.load();
			byte* wp = m_write_pointer;

			if (rp == wp)
			{
				m_thread_section.unlock();
				continue;
			}

			TaskInterface* task    = reinterpret_cast<TaskInterface*>(m_thread_read_pointer.load());
			const size_t task_size = task->size();
			rp                     = align_memory(rp + task_size, m_align);

			if (rp >= m_buffer + m_buffer_size)
				rp = m_buffer;

			m_thread_read_pointer = rp;
			m_thread_section.unlock();

			task->execute();

			// Submit executed task
			{
				m_thread_section.lock();
				rp = align_memory(m_read_pointer.load() + task_size, m_align);

				if (rp >= m_buffer + m_buffer_size)
					rp = m_buffer;

				m_read_pointer = rp;
				m_thread_section.unlock();
			}
		}
	}

	void ThreadManager::destroy_manager()
	{
		delete m_manager;
		m_manager = nullptr;
	}

	ThreadManager::ThreadManager()
	{
		PostDestroyController().push(destroy_manager);

		m_read_pointer        = m_buffer;
		m_thread_read_pointer = m_buffer;
		m_write_pointer       = m_buffer;
		resize(10);
	}

	ThreadManager::~ThreadManager()
	{
		m_running   = false;
		m_has_tasks = true;
		m_has_tasks.notify_all();

		for (auto& thread : m_threads)
		{
			if (thread.joinable())
			{
				thread.join();
			}
		}
	}
}// namespace Engine
