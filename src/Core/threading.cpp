#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/atomic.hpp>
#include <Core/etl/critical_section.hpp>
#include <Core/etl/deque.hpp>
#include <Core/etl/vector.hpp>
#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <Core/threading.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace Engine
{
	///////////////// TASK QUEUE IMPLEMENTATION /////////////////

	class TaskQueue
	{
	public:
		virtual void add_task_internal(Task::TaskImpl* task) = 0;
	};

	///////////////// TASK CLASS IMPLEMENTATION /////////////////

	class Task::TaskImpl
	{
	public:
		static Atomic<TaskImpl*> s_task_queue;

	public:
		Atomic<TaskImpl*> m_next = nullptr;

		union
		{
			byte* m_data = nullptr;
			Task::Function* m_function;
		};

		size_t m_data_size = 0;

		TaskQueue* m_queue = nullptr;
		Vector<Task> m_dependents;
		Atomic<uint32_t> m_dependencies = 0;
		Atomic<uint32_t> m_refs         = 1;
		Atomic<uint32_t> m_workers      = 0;
		Atomic<Status> m_status         = Undefined;
		Priority m_priority             = Middle;

		bool m_is_multi_threaded = false;

	public:
		static TaskImpl* allocate()
		{
			TaskImpl* head = s_task_queue.load(etl::memory_order_acquire);

			while (head)
			{
				TaskImpl* next = head->m_next.load(etl::memory_order_relaxed);

				if (s_task_queue.compare_exchange_weak(head, next, etl::memory_order_acquire, etl::memory_order_relaxed))
				{
					head->m_next.store(nullptr, etl::memory_order_relaxed);
					return head;
				}
			}

			return trx_new TaskImpl();
		}

		inline void reset()
		{
			m_queue = nullptr;
			m_dependents.clear();
			m_refs.store(1, etl::memory_order_relaxed);
			m_status            = Undefined;
			m_priority          = Middle;
			m_is_multi_threaded = false;

			m_function->~Function();
		}

		inline void add_ref() { m_refs.fetch_add(1, etl::memory_order_relaxed); }

		inline bool release()
		{
			if (m_refs.fetch_sub(1, etl::memory_order_acq_rel) == 1)
			{
				reset();

				TaskImpl* head = s_task_queue.load(etl::memory_order_relaxed);

				do
				{
					m_next.store(head, etl::memory_order_relaxed);
				} while (!s_task_queue.compare_exchange_weak(head, this, etl::memory_order_release, etl::memory_order_relaxed));

				return false;
			}

			return true;
		}

		inline void remove_dependency()
		{
			if (--m_dependencies == 0)
			{
				if (m_queue)
				{
					m_queue->add_task_internal(this);
				}
			}
		}

		inline bool execute()
		{
			m_function->execute();

			if (m_is_multi_threaded && m_workers.fetch_sub(1, std::memory_order_acq_rel) != 1)
			{
				m_status.store(Status::Completing, std::memory_order_release);
				return false;
			}
			else
			{
				m_status.store(Status::Executed, std::memory_order_release);
				for (Task& dependent : m_dependents)
				{
					dependent.m_impl->remove_dependency();
				}
				return true;
			}
		}

		inline void* data(size_t size)
		{
			if (m_data_size < size)
			{
				size = align_up(size, 64);

				ByteAllocator::deallocate(m_data);
				m_data      = ByteAllocator::allocate(size);
				m_data_size = size;
			}
			return m_data;
		}
	};

	Atomic<Task::TaskImpl*> Task::TaskImpl::s_task_queue;

	Task::Task() {}

	Task::Task(Priority priority) : m_impl(TaskImpl::allocate())
	{
		m_impl->m_priority = priority;
	}

	void* Task::data(size_t size)
	{
		return m_impl->data(size);
	}

	Task::Task(const Task& task) : m_impl(task.m_impl)
	{
		m_impl->add_ref();
	}

	Task::Task(Task&& task) : m_impl(task.m_impl)
	{
		task.m_impl = nullptr;
	}

	Task& Task::operator=(const Task& task)
	{
		if (this == &task)
			return *this;

		m_impl->release();
		m_impl = task.m_impl;
		m_impl->add_ref();
		return *this;
	}

	Task& Task::operator=(Task&& task)
	{
		if (this == &task)
			return *this;

		m_impl->release();
		m_impl      = task.m_impl;
		task.m_impl = nullptr;
		return *this;
	}

	Task::~Task()
	{
		if (m_impl)
			m_impl->release();
	}

	bool Task::is_multi_threaded() const
	{
		return m_impl->m_is_multi_threaded;
	}

	Task& Task::is_multi_threaded(bool flag)
	{
		m_impl->m_is_multi_threaded = flag;
		return *this;
	}

	Task& Task::add_dependent(const Task& dependent)
	{
		m_impl->m_dependents.push_back(dependent);
		dependent.m_impl->m_dependencies += 1;
		return *this;
	}

	Task& Task::add_dependent(Task&& dependent)
	{
		dependent.m_impl->m_dependencies += 1;
		m_impl->m_dependents.push_back(etl::move(dependent));
		return *this;
	}

	Task::Priority Task::priority() const
	{
		return m_impl->m_priority;
	}

	Task::Status Task::status() const
	{
		return m_impl->m_status;
	}

	///////////////// THREAD CLASS IMPLEMENTATION /////////////////

	static thread_local Thread* this_thread_instance = nullptr;

	struct Thread::Impl {
		Deque<Task::TaskImpl*> m_tasks[3];
		CriticalSection m_cs;

		struct ThreadData {
			std::thread m_thread;
			std::condition_variable m_cv;
			bool m_running = false;

			template<typename Func>
			ThreadData(Func&& func) : m_thread(etl::forward<Func>(func))
			{}

			~ThreadData()
			{
				m_running = false;
				m_cv.notify_all();

				if (m_thread.joinable())
				{
					m_thread.join();
				}
			}

		}* m_thread_data = nullptr;

		Impl() {}

		template<typename Func>
		Impl(Func&& func) : m_thread_data(trx_new ThreadData(etl::forward<Func>(func)))
		{}

		~Impl()
		{
			if (m_thread_data)
			{
				trx_delete m_thread_data;
				m_thread_data = nullptr;
			}
		}
	};

	Thread::Thread()
	{
		m_thread = trx_new Impl([this]() { thread_loop(); });
	}

	Thread::Thread(NoThread)
	{
		if (this_thread_instance != nullptr)
		{
			throw EngineException("Thread already exist!");
		}

		this_thread_instance = this;

		m_thread = trx_new Impl();
	}

	Thread::Thread(void (*function)(void* data), void* data)
	{
		m_thread = trx_new Impl([this, function, data]() {
			this_thread_instance = this;
			function(data);
			this_thread_instance = nullptr;
		});
	}

	void Thread::thread_loop()
	{
		this_thread_instance = this;

		Thread::Impl::ThreadData* thread = m_thread->m_thread_data;

		while (thread->m_running)
		{
			Task::TaskImpl* task = nullptr;
			{
				std::unique_lock lock(m_thread->m_cs);
				thread->m_cv.wait(lock, [this, thread] { return !thread->m_running || has_tasks(); });

				if (!thread->m_running)
					break;

				task = fetch_task_locked();
			}

			if (task)
			{
				task->m_status = Task::Executing;
				task->execute();
				task->release();

				execute();
			}
		}

		this_thread_instance = nullptr;
	}

	bool Thread::has_tasks() const
	{
		for (auto& queue : m_thread->m_tasks)
		{
			if (!queue.empty())
				return true;
		}
		return false;
	}

	Task::TaskImpl* Thread::fetch_task_locked()
	{
		for (uint32_t i = Task::High; i <= Task::Low; ++i)
		{
			auto& queue = m_thread->m_tasks[i];

			if (!queue.empty())
			{
				Task::TaskImpl* t = etl::move(queue.front());
				queue.pop_front();
				return t;
			}
		}
		return {};
	}

	Thread& Thread::execute()
	{
		if (this == static_self())
		{
			Task::TaskImpl* task = nullptr;
			while (true)
			{
				{
					ScopeLock lock(m_thread->m_cs);
					task = fetch_task_locked();
				}

				if (task)
				{
					task->m_status = Task::Executing;
					task->execute();
					task->release();
				}
				else
				{
					return *this;
				}
			}
		}
		return *this;
	}

	Thread& Thread::add_task(const Task& task)
	{
		{
			ScopeLock lock(m_thread->m_cs);
			task.m_impl->add_ref();
			m_thread->m_tasks[task.priority()].emplace_back(task.m_impl);
		}

		if (m_thread->m_thread_data)
		{
			m_thread->m_thread_data->m_cv.notify_one();
		}
		return *this;
	}

	Thread& Thread::add_task(Task&& task)
	{
		{
			ScopeLock lock(m_thread->m_cs);
			m_thread->m_tasks[task.priority()].emplace_back(task.m_impl);
		}

		task.m_impl = nullptr;

		if (m_thread->m_thread_data)
		{
			m_thread->m_thread_data->m_cv.notify_one();
		}
		return *this;
	}

	void Thread::static_sleep_for(float seconds)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(static_cast<size_t>(seconds * 1000000.f)));
	}

	Thread* Thread::static_self()
	{
		if (this_thread_instance == nullptr)
		{
			throw EngineException("Engine::ThisThread::self must be called from a thread that is registered by the engine");
		}

		return this_thread_instance;
	}

	void Thread::static_yield()
	{
		std::this_thread::yield();
	}

	Thread::~Thread()
	{
		if (m_thread)
		{
			trx_delete m_thread;
		}
	}

	///////////////// TASK GRAPH CLASS IMPLEMENTATION /////////////////

	class TaskGraphImpl : public TaskQueue
	{
	private:
		Vector<Thread*> m_workers;
		Deque<Task::TaskImpl*> m_tasks[3];

		std::mutex m_mutex;
		std::condition_variable m_cv;
		bool m_stop{false};

	private:
		inline bool is_empty()
		{
			for (uint32_t i = Task::High; i <= Task::Low; ++i)
			{
				if (!m_tasks[i].empty())
					return false;
			}
			return true;
		}

		Task::TaskImpl* acquire_task_locked(Deque<Task::TaskImpl*>& queue)
		{
			while (!queue.empty())
			{
				Task::TaskImpl* task = queue.front();
				Task::Status status  = task->m_status;

				if (task->m_is_multi_threaded)
				{
					if (status == Task::Status::Pending || status == Task::Status::Executing)
					{
						if (status == Task::Status::Pending)
							task->m_status = Task::Status::Executing;

						++task->m_workers;
						return task;
					}

					task->release();
					queue.pop_front();
				}
				else
				{
					queue.pop_front();

					if (status == Task::Status::Pending)
					{
						task->m_status = Task::Status::Executing;
						return task;
					}

					task->release();
				}
			}

			return nullptr;
		}

		Task::TaskImpl* acquire_task_locked()
		{
			for (uint32_t i = Task::High; i <= Task::Low; ++i)
			{
				auto& queue = m_tasks[i];

				if (Task::TaskImpl* task = acquire_task_locked(queue))
					return task;
			}

			return nullptr;
		}

		inline Task::TaskImpl* acquire_task()
		{
			std::unique_lock lock(m_mutex);
			return acquire_task_locked();
		}

		static void worker_main(void* self) { static_cast<TaskGraphImpl*>(self)->worker_loop(); }

		void worker_loop()
		{
			while (true)
			{
				Task::TaskImpl* task = nullptr;
				{
					std::unique_lock lock(m_mutex);
					m_cv.wait(lock, [this] { return m_stop || !is_empty(); });

					if (m_stop)
						return;

					task = acquire_task_locked();
				}

				if (task && task->execute())
				{
					task->release();
				}
			}
		}

	public:
		explicit TaskGraphImpl(size_t thread_count = std::thread::hardware_concurrency())
		{
			if (thread_count == 0)
				thread_count = 1;

			for (size_t i = 0; i < thread_count; ++i) m_workers.emplace_back(trx_new Thread(worker_main, this));
		}

		~TaskGraphImpl()
		{
			{
				std::unique_lock lock(m_mutex);
				m_stop = true;
			}
			m_cv.notify_all();
			for (Thread* worker : m_workers) trx_delete worker;
		}

		void add_task_locked(Task::TaskImpl* task)
		{
			task->add_ref();
			m_tasks[task->m_priority].push_back(task);
			task->m_status.store(Task::Pending, std::memory_order_release);

			if (task->m_is_multi_threaded)
			{
				task->add_ref();
				m_cv.notify_all();
			}
			else
			{
				m_cv.notify_one();
			}
		}

		void add_task_internal(Task::TaskImpl* task) override
		{
			std::unique_lock lock(m_mutex);
			add_task_locked(task);
		}

		void add_task(Task::TaskImpl* task)
		{
			if (task->m_queue != nullptr)
				return;

			std::unique_lock lock(m_mutex);

			if (task->m_dependencies == 0)
			{
				add_task_locked(task);
			}
			else
			{
				task->m_queue = this;
			}
		}

		void wait_for(Task::TaskImpl* task)
		{
			Task::TaskImpl* current = nullptr;
			{
				std::unique_lock lock(m_mutex);
				Task::Status status = task->m_status;

				if (status == Task::Status::Executed)
					return;

				if (task->m_queue == nullptr)
				{
					if (task->m_dependencies == 0)
					{
						add_task_locked(task);
					}
					else
					{
						task->m_queue = this;
					}
				}

				current = acquire_task_locked();
			}

			if (current && current->execute())
			{
				current->release();
			}

			while (task->m_status != Task::Status::Executed)
			{
				current = acquire_task();

				if (current && current->execute())
				{
					current->release();
				}
			}
		}
	};

	TaskGraph::TaskGraph() : m_impl(trx_new TaskGraphImpl()) {}

	TaskGraph::~TaskGraph()
	{
		trx_delete m_impl;

		Task::TaskImpl* impl = Task::TaskImpl::s_task_queue;

		while (impl)
		{
			Task::TaskImpl* next = impl->m_next;
			trx_delete impl;
			impl = next;
		}
	}

	TaskGraph* TaskGraph::instance()
	{
		static TaskGraph tg;
		return &tg;
	}

	TaskGraph& TaskGraph::add_task(const Task& task)
	{
		m_impl->add_task(task.m_impl);
		return *this;
	}

	TaskGraph& TaskGraph::wait_for(const Task& task)
	{
		m_impl->wait_for(task.m_impl);
		return *this;
	}

	///////////////// NAMED THREADS IMPLEMENTATION /////////////////

	static Thread* s_logic_thread = nullptr;

	class MainThread : public Thread
	{
	public:
		MainThread() : Thread(NoThread()) {}
		~MainThread() {}
	};

	ENGINE_EXPORT Thread* logic_thread()
	{
		return s_logic_thread;
	}

	ENGINE_EXPORT Thread* this_thread()
	{
		return Thread::static_self();
	}

	ENGINE_EXPORT void create_threads()
	{
		if (s_logic_thread == nullptr)
		{
			s_logic_thread = trx_new MainThread();
		}
	}

	template<typename ThreadType>
	static void destroy_thread(ThreadType*& thread)
	{
		if (thread)
		{
			trx_delete thread;
			thread = nullptr;
		}
	}

	ENGINE_EXPORT void destroy_threads()
	{
		destroy_thread(s_logic_thread);
	}
}// namespace Engine
