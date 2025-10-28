#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/atomic.hpp>
#include <Core/etl/deque.hpp>
#include <Core/etl/vector.hpp>
#include <Core/memory.hpp>
#include <Core/task_graph.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace Engine
{
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


	///////////////// TASK CLASS IMPLEMENTATION /////////////////

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

	///////////////// TASK GRAPH CLASS IMPLEMENTATION /////////////////

	class TaskGraphImpl
	{
	private:
		Vector<std::thread> m_workers;
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
			for (size_t i = 0; i < thread_count; ++i) m_workers.emplace_back([this] { worker_loop(); });
		}

		~TaskGraphImpl()
		{
			{
				std::unique_lock lock(m_mutex);
				m_stop = true;
			}
			m_cv.notify_all();
			for (auto& t : m_workers) t.join();
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
}// namespace Engine
