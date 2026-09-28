#include <Core/etl/atomic.hpp>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_timer.h>
#include <SDLPlatform/threading.hpp>

#if PLATFORM_LINUX
#include <pthread.h>
#elif PLATFORM_WINDOWS
#include <Core/etl/vector.hpp>
#include <windows.h>
#endif

namespace Trinex::Platform
{
	static thread_local Ref<Thread> s_current_thread;

	struct ThreadData {
		Thread* thread;
		void (*function)(void*);
		void* userdata;
		ThreadPriority priority;
		Atomic<bool> ready;
	};

	static int SDLCALL thread_main(void* userdata)
	{
		ThreadData* data = static_cast<ThreadData*>(userdata);

		s_current_thread = retain_ref(data->thread);
		SDLThreadSystem::instance()->priority(data->priority);

		auto function = data->function;
		void* args    = data->userdata;

		data->ready.store(true);
		data->ready.notify_one();

		if (function)
		{
			function(args);
		}

		trx_delete data;
		return 0;
	}

	SDLThread::~SDLThread()
	{
		if (m_thread)
		{
			SDL_DetachThread(m_thread);
		}
	}

	SDLThread& SDLThread::init(SDL_Thread* thread, ThreadPriority priority)
	{
		trinex_assert(m_thread == nullptr);

		m_thread   = thread;
		m_priority = priority;
		return *this;
	}

	u64 SDLThread::id() const
	{
		return m_thread ? SDL_GetThreadID(m_thread) : 0;
	}

	ThreadState SDLThread::state() const
	{
		if (m_thread == nullptr)
			return ThreadState::Complete;

		switch (SDL_GetThreadState(m_thread))
		{
			case SDL_THREAD_ALIVE: return ThreadState::Alive;
			case SDL_THREAD_DETACHED: return ThreadState::Detached;
			case SDL_THREAD_COMPLETE: return ThreadState::Complete;
			case SDL_THREAD_UNKNOWN:
			default: return ThreadState::Undefined;
		}
	}

	bool SDLThread::join(u64 nanoseconds)
	{
		if (m_thread == nullptr)
			return false;

		const u64 step = 1000000;
		u64 waited     = 0;

		while (SDL_GetThreadState(m_thread) == SDL_THREAD_ALIVE)
		{
			if (nanoseconds == 0 || waited >= nanoseconds)
				return false;

			const u64 remaining = nanoseconds - waited;
			const u64 delay     = remaining < step ? remaining : step;
			SDL_DelayNS(delay);
			waited += delay;
		}

		SDL_WaitThread(m_thread, nullptr);
		m_thread = nullptr;
		return true;
	}

	bool SDLThread::detach()
	{
		if (m_thread == nullptr)
			return false;

		SDL_DetachThread(m_thread);
		m_thread = nullptr;
		return true;
	}

	const char* SDLThread::name() const
	{
		return m_thread ? SDL_GetThreadName(m_thread) : nullptr;
	}

	ThreadPriority SDLThread::priority() const
	{
		return m_priority;
	}

	SDLThreadSystem::SDLThreadSystem() {}
	
	SDLThreadSystem* SDLThreadSystem::instance()
	{
		static SDLThreadSystem system;
		return &system;
	}

	Ref<Thread> SDLThreadSystem::create(const char* name, void (*function)(void*), void* userdata, ThreadPriority priority)
	{
		auto handle     = Ref<SDLThread>::make();
		ThreadData data = {handle.value(), function, userdata, priority, false};

		SDL_Thread* thread = SDL_CreateThread(thread_main, name, &data);

		if (thread == nullptr)
		{
			return {};
		}

		data.ready.wait(false);
		return handle;
	}

	Thread* SDLThreadSystem::current() const
	{
		return s_current_thread.value();
	}

	SDLThreadSystem& SDLThreadSystem::name(const char* name)
	{
#if PLATFORM_LINUX
		char buffer[16] = {};

		if (name)
		{
			for (usize i = 0; i < sizeof(buffer) - 1 && name[i] != '\0'; ++i)
			{
				buffer[i] = name[i];
			}
		}

		pthread_setname_np(pthread_self(), buffer);

#elif PLATFORM_WINDOWS

		if (name)
		{
			const int size = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);

			if (size > 0)
			{
				Vector<wchar_t> buffer(size);
				MultiByteToWideChar(CP_UTF8, 0, name, -1, buffer.data(), size);
				SetThreadDescription(GetCurrentThread(), buffer.data());
			}
		}
#endif

		return *this;
	}

	SDLThreadSystem& SDLThreadSystem::priority(ThreadPriority priority)
	{
		SDL_SetCurrentThreadPriority([priority]() {
			switch (priority)
			{
				case ThreadPriority::Low: return SDL_THREAD_PRIORITY_LOW;
				case ThreadPriority::High: return SDL_THREAD_PRIORITY_HIGH;
				case ThreadPriority::Critical: return SDL_THREAD_PRIORITY_TIME_CRITICAL;
				case ThreadPriority::Normal:
				default: return SDL_THREAD_PRIORITY_NORMAL;
			}
		}());

		return *this;
	}

	SDLThreadSystem& SDLThreadSystem::sleep(u64 nanoseconds)
	{
		SDL_DelayNS(nanoseconds);
		return *this;
	}

	SDLThreadSystem& SDLThreadSystem::yield()
	{
		SDL_DelayNS(0);
		return *this;
	}
}// namespace Trinex::Platform
