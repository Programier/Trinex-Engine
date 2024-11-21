#include <Core/memory.hpp>
#include <Core/thread.hpp>

namespace Engine
{
	static thread_local ThreadBase* this_thread_instance = nullptr;

#if PLATFORM_WINDOWS
	static constexpr inline std::size_t max_thread_name_length = 32;
#else
	static constexpr inline std::size_t max_thread_name_length = 16;
#endif

	ThreadBase::ThreadBase()
	{
		m_native_handle = pthread_self();

		update_id();
		update_name();
	}

	void ThreadBase::update_id()
	{
		std::unique_lock lock(m_edit_mutex);
		m_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
	}

	void ThreadBase::update_name()
	{
		std::unique_lock lock(m_edit_mutex);
		char thread_name[max_thread_name_length];
		if (pthread_getname_np(m_native_handle, thread_name, sizeof(thread_name)) == 0)
		{
			m_name = thread_name;
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
		return m_name;
	}

	ThreadBase& ThreadBase::name(const String& thread_name)
	{
		std::unique_lock lock(m_edit_mutex);
		m_name = thread_name.substr(0, std::min<size_t>(max_thread_name_length, static_cast<int>(thread_name.length())));
		pthread_setname_np(m_native_handle, m_name.c_str());
		return *this;
	}

	bool ThreadBase::is_destroyable()
	{
		return false;
	}

	Identifier ThreadBase::id()
	{
		return m_id;
	}

	ThreadBase::~ThreadBase()
	{}

	void Thread::thread_loop()
	{
		this_thread_instance = this;
		update_id();

		while (m_running)
		{
			while (m_running && m_read_pointer == m_write_pointer)
			{
				m_exec_flag.wait(false, std::memory_order_acquire);
			}

			if (!m_running)
				return;

			m_is_busy = true;
			byte* wp  = m_write_pointer;
			byte* rp  = m_read_pointer;

			while (wp != rp)
			{
				auto* task = reinterpret_cast<ExecutableObject*>(rp);
				auto size  = task->execute();

				rp = align_memory(rp + size, m_align);

				if (rp >= m_buffer + m_buffer_size)
					rp = m_buffer;

				m_read_pointer = rp;
			}

			m_is_busy = false;
			m_exec_flag.clear(std::memory_order_release);
			m_exec_flag.notify_all();
		}
	}

	Thread::Thread()
	{
		m_read_pointer  = m_buffer;
		m_write_pointer = m_buffer;

		m_thread        = std::thread([this]() { thread_loop(); });
		m_native_handle = m_thread.native_handle();

		update_name();
	}

	Thread::Thread(const String& thread_name) : Thread()
	{
		name(thread_name);
	}

	bool Thread::is_busy() const
	{
		return m_is_busy;
	}

	Thread& Thread::wait_all()
	{
		if (Thread::this_thread() != this)
		{
			while (is_busy() || (m_read_pointer != m_write_pointer))
			{
				m_exec_flag.wait(true, std::memory_order_release);
			}
		}
		return *this;
	}

	bool Thread::is_destroyable()
	{
		return true;
	}

	Thread::~Thread()
	{
		m_running = false;
		m_exec_flag.test_and_set(std::memory_order_release);
		m_exec_flag.notify_all();

		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}
}// namespace Engine
