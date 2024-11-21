#include <Core/engine_loading_controllers.hpp>
#include <Core/threading.hpp>

namespace Engine
{
	static MainThread* m_logic_thread = nullptr;
	static Thread* m_render_thread    = nullptr;

	ENGINE_EXPORT Thread* this_thread()
	{
		return ThisThread::self();
	}

	ENGINE_EXPORT Thread* render_thread()
	{
		return m_render_thread;
	}

	ENGINE_EXPORT MainThread* logic_thread()
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
			m_logic_thread = new MainThread();
		}

		if (m_render_thread == nullptr)
		{
			m_render_thread = new Thread();
		}
	}

	template<typename ThreadType>
	static void destroy_thread(ThreadType*& thread)
	{
		if (thread)
		{
			delete thread;
			thread = nullptr;
		}
	}

	ENGINE_EXPORT void destroy_threads()
	{
		destroy_thread(m_logic_thread);
		destroy_thread(m_render_thread);
	}
}// namespace Engine
