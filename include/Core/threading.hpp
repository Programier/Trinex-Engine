#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/atomic.hpp>
#include <Core/etl/deque.hpp>
#include <Core/etl/tuple.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/etl/utility.hpp>

namespace Engine
{
	class ENGINE_EXPORT Task final
	{
	public:
		enum Priority : uint8_t
		{
			High   = 0,
			Middle = 1,
			Low    = 2,
		};

		enum Status : uint8_t
		{
			Undefined,
			Pending,
			Executing,
			Completing,
			Executed
		};

	private:
		class TaskImpl;

		class Function
		{
		public:
			virtual void execute() = 0;
			virtual ~Function() {}
		};

		template<typename Callable, typename... Args>
		class FunctionImpl : public Function
		{
		private:
			Callable m_callable;
			Tuple<Args...> m_args;

			template<size_t... I>
			void call_impl(etl::index_sequence<I...>)
			{
				m_callable(etl::get<I>(m_args)...);
			}

		public:
			FunctionImpl(Callable&& callable, Args&&... args)
			    : m_callable(etl::forward<Callable>(callable)), m_args(etl::forward<Args>(args)...)
			{}

			void execute() override { call_impl(etl::index_sequence_for<Args...>{}); }
		};

	private:
		TaskImpl* m_impl = nullptr;

	private:
		Task(Priority priority);

		void* data(size_t size);

	public:
		Task();

		template<typename Callable, typename... Args, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Callable>, Task>>>
		Task(Callable&& callable, Args&&... args) : Task(Middle)
		{
			using FuncType = FunctionImpl<std::decay_t<Callable>, std::decay_t<Args>...>;
			new (data(sizeof(FuncType))) FuncType(etl::forward<Callable>(callable), etl::forward<Args>(args)...);
		}

		template<typename Callable, typename... Args, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Callable>, Task>>>
		Task(Priority priority, Callable&& callable, Args&&... args) : Task(priority)
		{
			using FuncType = FunctionImpl<std::decay_t<Callable>, std::decay_t<Args>...>;
			new (data(sizeof(FuncType))) FuncType(etl::forward<Callable>(callable), etl::forward<Args>(args)...);
		}

		Task(const Task& task);
		Task(Task&& task);
		Task& operator=(const Task& task);
		Task& operator=(Task&& task);
		~Task();

		static byte worker_index();
		Task& max_threads(byte count);
		Task& add_dependent(const Task& dependent);
		Task& add_dependent(Task&& dependent);
		Priority priority() const;
		Status status() const;

		inline bool is_valid() const { return m_impl != nullptr; }
		inline bool is_pending() const { return status() == Pending; }
		inline bool is_executed() const { return status() == Executed; }
		inline bool is_executing() const { return status() == Executing; }

		inline operator bool() const { return is_valid(); }

		friend class TaskQueue;
		friend class TaskGraph;
		friend class TaskGraphImpl;
		friend class Thread;
	};

	class ENGINE_EXPORT Thread
	{
	public:
		struct NoThread {
		};

	private:
		struct Impl;
		Impl* m_thread = nullptr;

	private:
		void thread_loop();
		Task::TaskImpl* fetch_task_locked();

		bool has_tasks() const;

	public:
		Thread();
		Thread(NoThread);
		Thread(void (*function)(void* data), void* data = nullptr);

		size_t execute();
		bool execute_once();
		Thread& add_task(const Task& task);
		Thread& add_task(Task&& task);

		static void static_sleep_for(float seconds);
		static Thread* static_self();
		static void static_yield();

		~Thread();
	};

	class ENGINE_EXPORT TaskGraph final
	{
	private:
		class TaskGraphImpl* m_impl;

		TaskGraph();
		~TaskGraph();

	public:
		static TaskGraph* instance();

		byte workers() const;
		TaskGraph& add_task(const Task& task);
		TaskGraph& wait_for(const Task& task);


		template<typename Callable>
		inline TaskGraph& for_each(size_t count, Callable&& func, size_t block = 64, byte max_threads = 255)
		{
			if (count < block)
			{
				for (size_t i = 0; i < count; ++i)
				{
					func(i);
				}
				return *this;
			}

			Atomic<size_t> counter = 0;

			Task task = Task(Task::High, [&]() {
				size_t start;

				while ((start = counter.fetch_add(block)) < count)
				{
					size_t end = start + block;

					if (end > count)
						end = count;

					for (size_t i = start; i < end; ++i)
					{
						func(i);
					}
				}
			});

			task.max_threads(max_threads);
			return wait_for(task);
		}
	};

	ENGINE_EXPORT void create_threads();
	ENGINE_EXPORT void destroy_threads();
	ENGINE_EXPORT Thread* logic_thread();
	ENGINE_EXPORT Thread* this_thread();

	inline bool is_in_logic_thread()
	{
		return this_thread() == logic_thread();
	}
}// namespace Engine
