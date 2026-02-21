#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/atomic.hpp>
#include <Core/etl/critical_section.hpp>
#include <Core/etl/deque.hpp>
#include <Core/etl/scope_variable.hpp>
#include <Core/etl/vector.hpp>
#include <Core/memory.hpp>
#include <Core/threading.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace Engine
{
	///////////////// GLOBAL VARIABLES /////////////////

	static thread_local byte s_worker_idx = 0;

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
		CriticalSection m_cs;

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
		Atomic<Status> m_status         = Undefined;

		Priority m_priority = Middle;
		byte m_workers      = 0;
		byte m_max_threads  = 1;

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
			m_status      = Undefined;
			m_priority    = Middle;
			m_max_threads = 1;

			m_function->~Function();
		}

		inline void add_ref(uint32_t count = 1) { m_refs.fetch_add(count, etl::memory_order_relaxed); }

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

		inline bool execute(byte worker = 0)
		{
			{
				ScopeVariable scope(s_worker_idx, worker);
				m_function->execute();
			}

			if (m_max_threads > 1)
			{
				ScopeLock lock(m_cs);

				if (m_workers-- == 1)
				{
					m_status.store(Status::Executed, etl::memory_order_release);

					for (Task& dependent : m_dependents)
					{
						dependent.m_impl->remove_dependency();
					}

					return true;
				}
				else
				{
					m_status.store(Status::Completing, etl::memory_order_release);
					return false;
				}
			}
			else
			{
				trinex_assert(m_workers-- == 0);
				m_status.store(Status::Executed, etl::memory_order_release);

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

	byte Task::worker_index()
	{
		return s_worker_idx;
	}

	Task& Task::max_threads(byte count)
	{
		m_impl->m_max_threads = count ? count : 1;
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
		return m_impl->m_status.load();
	}

	///////////////// THREAD CLASS IMPLEMENTATION /////////////////

	static thread_local Thread* this_thread_instance = nullptr;

	struct Thread::Impl {
		Deque<Task::TaskImpl*> m_tasks[3];
		CriticalSection m_cs;

		struct ThreadData {
			std::thread* m_thread;
			std::condition_variable m_cv;
			bool m_running = true;

			template<typename Func>
			void start(Func&& func)
			{
				m_thread = trx_new std::thread(etl::forward<Func>(func));
			}

			~ThreadData()
			{
				m_running = false;
				m_cv.notify_all();

				if (m_thread->joinable())
				{
					m_thread->join();
				}

				trx_delete m_thread;
			}

		}* m_thread_data = nullptr;

		Impl() {}

		template<typename Func>
		Impl(Func&& func) : m_thread_data(trx_new ThreadData())
		{
			m_thread_data->start(etl::forward<Func>(func));
		}

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
		trinex_assert_msg(this_thread_instance == nullptr, "Thread already exist");
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

	size_t Thread::execute()
	{
		size_t count = 0;

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
					++count;
				}
				else
				{
					return count;
				}
			}
		}

		return count;
	}

	bool Thread::execute_once()
	{
		if (this == static_self())
		{
			Task::TaskImpl* task = nullptr;

			{
				ScopeLock lock(m_thread->m_cs);
				task = fetch_task_locked();
			}

			if (task)
			{
				task->m_status = Task::Executing;
				task->execute();
				task->release();
				return true;
			}
			else
			{
				return false;
			}
		}

		return false;
	}

	Thread& Thread::add_task(const Task& task)
	{
		{
			ScopeLock lock(m_thread->m_cs);
			task.m_impl->add_ref();
			task.m_impl->m_max_threads = 1;
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
		trinex_assert_msg(this_thread_instance, "self method must be called from a thread that is registered by the engine");
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

		Task::TaskImpl* acquire_task_locked(Deque<Task::TaskImpl*>& queue, byte& worker)
		{
			while (!queue.empty())
			{
				Task::TaskImpl* task       = queue.front();
				const uint32_t max_threads = task->m_max_threads;

				if (max_threads > 1)
				{
					ScopeLock lock(task->m_cs);

					if (task->m_workers < max_threads)
					{
						Task::Status status = task->m_status.load(etl::memory_order_acquire);

						if (status == Task::Status::Executed || status == Task::Status::Completing)
						{
							queue.pop_front();
							task->release();
							continue;
						}

						worker = task->m_workers++;

						if (status == Task::Status::Pending)
							task->m_status = Task::Status::Executing;

						return task;
					}

					queue.pop_front();
					task->release();
				}
				else
				{
					queue.pop_front();
					task->release();

					Task::Status status = Task::Status::Pending;

					if (task->m_status.compare_exchange_weak(status, Task::Status::Executing))
					{
						task->m_status.store(Task::Status::Executing, etl::memory_order_release);
						task->m_workers = 1;
						worker          = 0;
						return task;
					}
				}
			}
			return nullptr;
		}

		Task::TaskImpl* acquire_task_locked(byte& worker)
		{
			for (uint32_t i = Task::High; i <= Task::Low; ++i)
			{
				if (Task::TaskImpl* task = acquire_task_locked(m_tasks[i], worker))
					return task;
			}

			return nullptr;
		}

		inline Task::TaskImpl* acquire_task(byte& worker)
		{
			if (m_mutex.try_lock())
			{
				Task::TaskImpl* task = acquire_task_locked(worker);
				m_mutex.unlock();
				return task;
			}

			return nullptr;
		}

		static void worker_main(void* self) { static_cast<TaskGraphImpl*>(self)->worker_loop(); }

		void worker_loop()
		{
			while (true)
			{
				byte worker;
				Task::TaskImpl* task = nullptr;
				{
					std::unique_lock<std::mutex> lock(m_mutex);
					m_cv.wait(lock, [this] { return m_stop || !is_empty(); });

					if (m_stop)
						return;

					task = acquire_task_locked(worker);
				}

				if (task == nullptr)
					continue;

				if (task && task->execute(worker))
				{
					task->release();
				}
			}
		}

	public:
		explicit TaskGraphImpl(size_t thread_count = 10)
		{
			if (thread_count == 0)
				thread_count = 1;

			m_workers.reserve(thread_count);
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
			task->m_queue = this;
			task->m_status.store(Task::Status::Pending, etl::memory_order_release);

			task->add_ref(2);

			m_tasks[task->m_priority].push_back(task);

			if (task->m_max_threads > 1)
				m_cv.notify_all();
			else
				m_cv.notify_one();
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
			byte worker;
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

				current = acquire_task_locked(worker);
			}

			if (current && current->execute(worker))
			{
				current->release();
			}

			while (task->m_status.load(etl::memory_order_acquire) != Task::Status::Executed)
			{
				current = acquire_task(worker);

				if (current && current->execute(worker))
				{
					current->release();
				}
			}
		}

		byte workers() const { return m_workers.size(); }
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

	byte TaskGraph::workers() const
	{
		return m_impl->workers();
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
