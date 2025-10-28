#include <Core/engine_loading_controllers.hpp>
#include <Core/threading.hpp>

namespace Engine
{
	static Thread* s_logic_thread = nullptr;

	class MainThread : public Thread
	{
	public:
		MainThread() : Thread(NoThread())
		{
			register_thread();
			register_thread_name("Main");
		}
		~MainThread() { unregister_thread(); }
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
