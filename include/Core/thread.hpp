#pragma once
#include <Core/engine_types.hpp>
#include <Core/exception.hpp>
#include <Core/executable_object.hpp>
#include <Core/memory.hpp>
#include <mutex>
#include <thread>

namespace Engine
{
	class ENGINE_EXPORT ThreadBase
	{
	protected:
		Identifier m_id;
		String m_name;
		std::recursive_mutex m_edit_mutex;
		std::thread::native_handle_type m_native_handle;

		ThreadBase();

		void update_id();
		void update_name();


	public:
		static void sleep_for(float seconds);
		static ThreadBase* this_thread();

		const String& name() const;
		ThreadBase& name(const String& thread_name);
		virtual bool is_destroyable();
		Identifier id();
		virtual ~ThreadBase();
	};


	class ENGINE_EXPORT Thread : public ThreadBase
	{
	private:
		template<typename Func>
		struct FunctionCaller : public ExecutableObject {
			std::decay_t<Func> m_func;

		public:
			FunctionCaller(Func&& func) : m_func(std::forward<Func>(func))
			{}

			int_t execute() override
			{
				m_func();
				return sizeof(*this);
			}
		};

		struct SkipBytes : public ExecutableObject {
			int_t m_bytes;
			SkipBytes(int_t bytes) : m_bytes(bytes)
			{}

			int_t execute() override
			{
				return m_bytes;
			}
		};

		static constexpr inline size_t m_buffer_size = 1024 * 1024 * 1;
		static constexpr inline size_t m_align       = 16;

		alignas(m_align) byte m_buffer[m_buffer_size];
		std::thread m_thread;

		Atomic<byte*> m_read_pointer;
		Atomic<byte*> m_write_pointer;

		Atomic<bool> m_running = true;
		Atomic<bool> m_is_busy = false;

		std::atomic_flag m_exec_flag      = ATOMIC_FLAG_INIT;
		std::atomic_flag m_push_task_flag = ATOMIC_FLAG_INIT;

		void thread_loop();

		inline size_t free_size() const
		{
			byte* rp = m_read_pointer;
			byte* wp = m_write_pointer;

			if (rp < wp)
			{
				return (rp - m_buffer) + ((m_buffer + m_buffer_size) - wp);
			}
			else if (rp == wp)
			{
				return m_buffer_size;
			}
			else
			{
				return rp - wp;
			}
		}

		FORCE_INLINE void wait_for_size(size_t size)
		{
			while (free_size() < size)
			{
				std::this_thread::yield();
			}
		}

	public:
		Thread();
		Thread(const String& name);

		bool is_busy() const;
		Thread& wait_all();
		bool is_destroyable() override;

		template<typename CommandType, typename... Args>
		inline Thread& insert_new_task(Args&&... args)
		{
			while (m_push_task_flag.test_and_set(std::memory_order_acquire))
			{
				std::this_thread::yield();
			}

			size_t task_size = sizeof(CommandType);
			byte* wp         = m_write_pointer;

			if (wp + task_size > m_buffer + m_buffer_size)
			{
				wait_for_size(sizeof(SkipBytes));
				new (wp) SkipBytes((m_buffer + m_buffer_size) - wp);
				wp = m_buffer;
			}

			wait_for_size(task_size);

			new (wp) CommandType(std::forward<Args>(args)...);
			wp = align_memory(wp + task_size, m_align);

			if (wp >= m_buffer + m_buffer_size)
			{
				wp = m_buffer;
			}

			m_write_pointer = wp;
			m_exec_flag.test_and_set(std::memory_order_release);
			m_push_task_flag.clear(std::memory_order_release);
			return *this;
		}

		template<typename Function>
		FORCE_INLINE Thread& call_function(Function&& function)
		{
			return insert_new_task<FunctionCaller<Function>>(std::forward<Function>(function));
		}

		template<typename Function, typename... Args>
		FORCE_INLINE Thread& call_function(Function&& function, Args&&... args)
		{
			auto new_function = std::bind(std::forward<Function>(function), std::forward<Args>(args)...);
			return insert_new_task<FunctionCaller<decltype(new_function)>>(std::move(new_function));
		}

		virtual ~Thread();
	};
}// namespace Engine
