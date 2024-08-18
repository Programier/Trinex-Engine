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

	SkipThreadCommand::SkipThreadCommand(size_t bytes) : m_skip_bytes(bytes)
	{}

	int_t SkipThreadCommand::execute()
	{
		return static_cast<int>(m_skip_bytes);
	}

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
		m_name =
				thread_name.substr(0, std::min<size_t>(max_thread_name_length, static_cast<int>(thread_name.length())));
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

	void Thread::thread_loop(Thread* self)
	{
		this_thread_instance = self;
		self->update_id();
		while (!self->m_is_shuting_down.load())
		{
			bool contains_tasks = false;

			{
				std::unique_lock lock(self->m_exec_mutex);

				self->m_is_thread_busy.store(false);
				self->m_wait_cv.notify_all();
				using namespace std::chrono_literals;

				if (self->m_exec_cv.wait_until(lock, std::chrono::system_clock::now() + 1ms, [self]() -> bool {
						return self->m_command_buffer.unreaded_buffer_size() != 0 || self->m_is_shuting_down;
					}))
				{
					self->m_is_thread_busy.store(true);
					contains_tasks = true;
				}

				if (self->m_is_shuting_down.load())
					return;
			}

			if (contains_tasks)
			{
				ExecutableObject* object = nullptr;

				while ((object = reinterpret_cast<ExecutableObject*>(self->m_command_buffer.reading_pointer())))
				{
					int_t size = object->execute();
					std::destroy_at(object);
					self->m_command_buffer.finish_read(size);
				}
			}
		}

		self->m_wait_cv.notify_all();
	}


	int_t Thread::NewTaskEvent::execute()
	{
		return 0;
	}


	Thread::Thread(size_t size)
	{
		m_is_shuting_down.store(false);
		m_event.m_thread = this;

		m_command_buffer.init(size, 16, &m_event);
		m_thread		= new std::thread(&Thread::thread_loop, this);
		m_native_handle = m_thread->native_handle();

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
		return m_is_thread_busy.load();
	}

	Thread& Thread::wait_all()
	{
		if (Thread::this_thread() != this)
		{
			std::unique_lock lock(m_wait_mutex);

			m_wait_cv.wait(lock, [this]() -> bool {
				return m_is_shuting_down || (is_thread_sleep() && m_command_buffer.unreaded_buffer_size() == 0);
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
		return m_command_buffer;
	}

	Thread::~Thread()
	{
		m_is_shuting_down.store(true);
		m_exec_cv.notify_all();

		m_thread->join();
		delete m_thread;
	}
}// namespace Engine
