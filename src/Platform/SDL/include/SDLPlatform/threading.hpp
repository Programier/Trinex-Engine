#pragma once
#include <Platform/threading.hpp>

struct SDL_Thread;

namespace Trinex::Platform
{
	class SDLThread : public Thread
	{
	private:
		SDL_Thread* m_thread      = nullptr;
		ThreadPriority m_priority = ThreadPriority::Normal;

	public:
		~SDLThread();

		SDLThread& init(SDL_Thread* thread, ThreadPriority priority);

		u64 id() const override;
		ThreadState state() const override;
		bool join(u64 nanoseconds) override;
		bool detach() override;
		const char* name() const override;
		ThreadPriority priority() const override;
	};

	class SDLThreadSystem final : public ThreadSystem
	{
	public:
		static SDLThreadSystem* instance();

		SDLThreadSystem();
		Ref<Thread> create(const char* name, void (*function)(void*), void* userdata = nullptr,
		                   ThreadPriority priority = ThreadPriority::Normal) override;

		Thread* current() const override;
		SDLThreadSystem& name(const char* name) override;
		SDLThreadSystem& priority(ThreadPriority priority) override;
		SDLThreadSystem& sleep(u64 nanoseconds) override;
		SDLThreadSystem& yield() override;
	};
}// namespace Trinex::Platform
