#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/atomic.hpp>
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

		Vector<Task> m_deps;
		Atomic<uint64_t> m_refs = 1;
		Atomic<Status> m_status = Undefined;
		Priority m_priority     = Middle;

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
			m_deps.clear();
			m_refs.store(1, etl::memory_order_relaxed);
			m_status   = Undefined;
			m_priority = Middle;

			m_function->~Function();
		}

		inline void add_ref() { m_refs.fetch_add(1, etl::memory_order_relaxed); }

		inline void release()
		{
			if (m_refs.fetch_sub(1, etl::memory_order_acq_rel) == 1)
			{
				reset();

				TaskImpl* head = s_task_queue.load(etl::memory_order_relaxed);

				do
				{
					m_next.store(head, etl::memory_order_relaxed);
				} while (!s_task_queue.compare_exchange_weak(head, this, etl::memory_order_release, etl::memory_order_relaxed));
			}
		}

		bool ready() const
		{
			for (const auto& dep : m_deps)
				if (dep.m_impl->m_status != Status::Executed)
					return false;
			return true;
		}

		inline void execute()
		{
			m_function->execute();
			m_status.store(Status::Executed, etl::memory_order_release);
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

	Task& Task::clear_dependencies()
	{
		m_impl->m_deps.clear();
		return *this;
	}

	Task& Task::add_dependency(const Task& dep)
	{
		m_impl->m_deps.push_back(dep);
		return *this;
	}

	Task& Task::add_dependency(Task&& dep)
	{
		m_impl->m_deps.push_back(etl::move(dep));
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
		std::thread thread;
		std::condition_variable cv;
		bool running = false;

		template<typename... Args>
		Impl(Args... args) : thread(args...)
		{}
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

		while (m_thread->running)
		{
			Task::TaskImpl* task = nullptr;
			{
				std::unique_lock lock(m_cs);
				m_thread->cv.wait(lock, [this] { return !m_thread->running || has_tasks(); });

				if (!m_thread->running)
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

	Task::TaskImpl* Thread::fetch_task_locked()
	{
		for (uint32_t i = Task::High; i <= Task::Low; ++i)
		{
			auto& queue = m_tasks[i];

			while (!queue.empty())
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
					ScopeLock lock(m_cs);
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
			ScopeLock lock(m_cs);
			task.m_impl->add_ref();
			m_tasks[task.priority()].emplace_back(task.m_impl);
		}

		if (m_thread)
		{
			m_thread->cv.notify_one();
		}
		return *this;
	}

	Thread& Thread::add_task(Task&& task)
	{
		{
			ScopeLock lock(m_cs);
			m_tasks[task.priority()].emplace_back(task.m_impl);
		}

		task.m_impl = nullptr;

		if (m_thread)
		{
			m_thread->cv.notify_one();
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
			m_thread->running = false;
			m_thread->cv.notify_all();

			if (m_thread->thread.joinable())
			{
				m_thread->thread.join();
			}

			trx_delete m_thread;
		}
	}

	///////////////// TASK GRAPH CLASS IMPLEMENTATION /////////////////

	class TaskGraphImpl
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

		Task::TaskImpl* fetch_task_locked()
		{
			for (uint32_t i = Task::High; i <= Task::Low; ++i)
			{
				auto& queue = m_tasks[i];

				while (!queue.empty())
				{
					Task::TaskImpl* t = queue.front();
					queue.pop_front();

					if (t->m_status == Task::Status::Pending)
						return t;
				}
			}

			return nullptr;
		}

		bool execute_task(Task::TaskImpl* task)
		{
			if (task->ready())
			{
				Task::Status expected = Task::Status::Pending;

				if (task->m_status.compare_exchange_strong(expected, Task::Status::Executing))
				{
					task->execute();
					return true;
				}

				return expected == Task::Executed;
			}
			else
			{
				push_back(task);
			}

			return false;
		}

		bool execute_next()
		{
			Task::TaskImpl* task = nullptr;
			{
				std::unique_lock lock(m_mutex);
				task = fetch_task_locked();
			}

			if (task)
			{
				if (execute_task(task))
				{
					task->release();
				}

				return true;
			}

			return false;
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

					task = fetch_task_locked();
				}

				if (task && execute_task(task))
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

		void push_back(Task::TaskImpl* task)
		{
			std::unique_lock lock(m_mutex);
			m_tasks[task->m_priority].push_back(task);
			task->m_status.store(Task::Pending, std::memory_order_release);
			m_cv.notify_one();
		}

		void push_front(Task::TaskImpl* task)
		{
			std::unique_lock lock(m_mutex);
			m_tasks[task->m_priority].push_front(task);
			task->m_status.store(Task::Pending, std::memory_order_release);
			m_cv.notify_one();
		}

		void wait_for(Task::TaskImpl* task)
		{
			if (!task)
				return;

			for (const Task& dep : task->m_deps) wait_for(dep.m_impl);

			while (true)
			{
				Task::Status st = task->m_status;

				if (st == Task::Status::Executed)
					return;

				if (st == Task::Status::Pending && task->ready())
				{
					Task::Status expected = Task::Status::Pending;

					if (task->m_status.compare_exchange_strong(expected, Task::Status::Executing))
					{
						task->execute();
						return;
					}
				}

				if (!execute_next())
				{
					std::this_thread::yield();
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
		task.m_impl->add_ref();
		m_impl->push_back(task.m_impl);
		return *this;
	}

	TaskGraph& TaskGraph::add_task(Task&& task)
	{
		m_impl->push_back(task.m_impl);
		task.m_impl = nullptr;
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
