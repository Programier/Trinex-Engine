#include <Core/memory.hpp>
#include <Core/thread.hpp>
#include <Core/threading.hpp>

namespace Engine
{
	static thread_local Thread* this_thread_instance = nullptr;

	Thread::Thread(NoThreadContext ctx)
	{
		m_read_pointer  = m_buffer;
		m_write_pointer = m_buffer;
	}

	Thread::Thread() : Thread(NoThreadContext{})
	{
		m_thread = new std::thread([this]() { thread_loop(); });
	}

	Thread& Thread::execute_commands()
	{
		m_is_busy = true;
		byte* wp  = m_write_pointer;
		byte* rp  = m_read_pointer;

		while (wp != rp)
		{
			auto* task = reinterpret_cast<TaskInterface*>(rp);
			auto size  = task->size();
			task->execute();
			std::destroy_at(task);

			rp = align_memory(rp + size, m_align);

			if (rp >= m_buffer + m_buffer_size)
				rp = m_buffer;

			m_read_pointer = rp;
		}

		m_is_busy = false;
		m_exec_flag.clear();
		m_exec_flag.notify_all();

		return *this;
	}

	void Thread::thread_loop()
	{
		this_thread_instance = this;

		while (m_running)
		{
			while (m_running && m_read_pointer == m_write_pointer)
			{
				m_exec_flag.wait(false);
			}

			if (!m_running)
				return;

			execute_commands();
		}
	}

	bool Thread::is_busy() const
	{
		return m_is_busy;
	}

	Thread& Thread::wait_all()
	{
		if (ThisThread::self() != this)
		{
			while (is_busy() || (m_read_pointer != m_write_pointer))
			{
				m_exec_flag.wait(true);
			}
		}
		return *this;
	}

	Thread::~Thread()
	{
		m_running = false;
		m_exec_flag.test_and_set();
		m_exec_flag.notify_all();

		if (m_thread)
		{
			if (m_thread->joinable())
			{
				m_thread->join();
			}

			delete m_thread;
		}
	}


	Thread::NoThreadContext MainThread::no_thead_context()
	{
		if (logic_thread() != nullptr)
			throw EngineException("Cannot create main thread, because an object of this class already exists");
		return {};
	}

	MainThread::MainThread() : Thread(no_thead_context())
	{

		this_thread_instance = this;
	}

	namespace ThisThread
	{
		ENGINE_EXPORT void sleep_for(float seconds)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(static_cast<size_t>(seconds * 1000000.f)));
		}

		ENGINE_EXPORT Thread* self()
		{
			if (this_thread_instance == nullptr)
			{
				throw EngineException("Engine::ThisThread::self must be called from a thread that is registered by the engine");
			}

			return this_thread_instance;
		}
	}// namespace ThisThread
}// namespace Engine
