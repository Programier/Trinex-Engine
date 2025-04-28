#include <Core/engine_loading_controllers.hpp>
#include <Core/threading.hpp>

namespace Engine
{
	static CommandBufferThread* m_logic_thread  = nullptr;
	static CommandBufferThread* m_render_thread = nullptr;

	class MainThread : public CommandBufferThread
	{
	public:
		MainThread() : CommandBufferThread(NoThread())
		{
			register_thread();
			register_thread_name("Main");
		}
		~MainThread() { unregister_thread(); }
	};

	ENGINE_EXPORT Thread* this_thread()
	{
		return ThisThread::self();
	}

	ENGINE_EXPORT CommandBufferThread* render_thread()
	{
		return m_render_thread;
	}

	ENGINE_EXPORT CommandBufferThread* logic_thread()
	{
		return m_logic_thread;
	}

	ENGINE_EXPORT bool is_in_render_thread()
	{
		return this_thread() == render_thread();
	}

	ENGINE_EXPORT bool is_in_logic_thread()
	{
		return this_thread() == m_logic_thread;
	}

	ENGINE_EXPORT void create_threads()
	{
		if (m_logic_thread == nullptr)
		{
			m_logic_thread = allocate<MainThread>();
		}

		if (m_render_thread == nullptr)
		{
			m_render_thread = allocate<CommandBufferThread>("Render");
		}
	}

	template<typename ThreadType>
	static void destroy_thread(ThreadType*& thread)
	{
		if (thread)
		{
			release(thread);
			thread = nullptr;
		}
	}

	ENGINE_EXPORT void destroy_threads()
	{
		destroy_thread(m_logic_thread);
		destroy_thread(m_render_thread);
	}
}// namespace Engine
