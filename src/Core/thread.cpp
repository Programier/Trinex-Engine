#include <Core/memory.hpp>
#include <Core/thread.hpp>
#include <Core/threading.hpp>

namespace Engine
{
	static thread_local Thread* this_thread_instance = nullptr;

	struct WaitTask : public Task<WaitTask> {
		CriticalSection* section;

		WaitTask(CriticalSection* section) : section(section) {}

		void execute() override { section->unlock(); }
	};

	Thread& Thread::register_thread_name(const String& name)
	{
		return *this;
	}

	Thread& Thread::register_thread()
	{
		this_thread_instance = this;
		return *this;
	}

	Thread& Thread::unregister_thread()
	{
		this_thread_instance = nullptr;
		return *this;
	}

	Thread::Thread(NoThread, size_t command_buffer_size)
	    : m_buffer(ByteAllocator::allocate(command_buffer_size)), m_buffer_size(command_buffer_size)
	{
		m_read_pointer  = m_buffer;
		m_write_pointer = m_buffer;
	}

	Thread::Thread(const char* name, size_t command_buffer_size) : Thread(NoThread(), command_buffer_size)
	{
		m_thread = new std::thread([this]() { thread_loop(); });
	}

	void Thread::thread_loop()
	{
		register_thread();

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

		unregister_thread();
	}

	Thread& Thread::execute_commands()
	{
		if (ThisThread::self() == this)
		{
			byte* wp = m_write_pointer;
			byte* rp = m_read_pointer;

			while (wp != rp)
			{
				auto* task = reinterpret_cast<TaskInterface*>(rp);
				auto size  = task->size();
				task->execute();
				std::destroy_at(task);

				rp = align_memory(rp + size, command_alignment);

				if (rp >= m_buffer + m_buffer_size)
					rp = m_buffer;

				m_read_pointer = rp;
			}

			m_exec_flag.clear();
			m_exec_flag.notify_all();
		}
		return *this;
	}

	Thread& Thread::wait()
	{
		if (ThisThread::self() != this)
		{
			CriticalSection m_section;
			m_section.lock();
			create_task<WaitTask>(&m_section);
			m_section.lock();
		}
		else
		{
			execute_commands();
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

		ByteAllocator::deallocate(m_buffer);
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
