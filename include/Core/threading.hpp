#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/atomic.hpp>
#include <Core/etl/critical_section.hpp>
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

		template<typename Callable, typename... Args>
		Task(Callable&& callable, Args&&... args) : Task(Middle)
		{
			using FuncType = FunctionImpl<std::decay_t<Callable>, std::decay_t<Args>...>;
			new (data(sizeof(FuncType))) FuncType(etl::forward<Callable>(callable), etl::forward<Args>(args)...);
		}

		template<typename Callable, typename... Args>
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

		Task& clear_dependencies();
		Task& add_dependency(const Task& dep);
		Task& add_dependency(Task&& dep);
		Priority priority() const;
		Status status() const;

		inline bool is_valid() const { return m_impl != nullptr; }
		inline bool is_pending() const { return status() == Pending; }
		inline bool is_executed() const { return status() == Executed; }
		inline bool is_executing() const { return status() == Executing; }

		inline operator bool() const { return is_valid(); }

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

		Deque<Task::TaskImpl*> m_tasks[3];
		Impl* m_thread = nullptr;

		CriticalSection m_cs;

	private:
		void thread_loop();
		Task::TaskImpl* fetch_task_locked();

		inline bool has_tasks() const
		{
			for (auto& queue : m_tasks)
			{
				if (!queue.empty())
					return true;
			}
			return false;
		}

	public:
		Thread();
		Thread(NoThread);
		Thread(void (*function)(void* data), void* data = nullptr);

		Thread& execute();
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

		TaskGraph& add_task(const Task& task);
		TaskGraph& add_task(Task&& task);
		TaskGraph& wait_for(const Task& task);
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
