#pragma once
#include <Core/ref_counted.hpp>
#include <Platform/enums.hpp>
#include <Platform/types.hpp>

namespace Trinex::Platform
{
	class ENGINE_EXPORT Thread : public RefCounted
	{
	public:
		virtual u64 id() const                  = 0;
		virtual ThreadState state() const       = 0;
		virtual bool join(u64 nanoseconds)      = 0;
		virtual bool detach()                   = 0;
		virtual const char* name() const        = 0;
		virtual ThreadPriority priority() const = 0;
	};

	class ENGINE_EXPORT ThreadSystem
	{
	public:
		static ThreadSystem* instance();

		virtual ~ThreadSystem() = default;

		virtual Ref<Thread> create(const char* name, void (*function)(void*), void* userdata = nullptr,
		                           ThreadPriority priority = ThreadPriority::Normal) = 0;

		virtual Thread* current() const                         = 0;
		virtual ThreadSystem& name(const char* name)            = 0;
		virtual ThreadSystem& priority(ThreadPriority priority) = 0;
		virtual ThreadSystem& sleep(u64 nanoseconds)            = 0;
		virtual ThreadSystem& yield()                           = 0;

		inline Ref<Thread> create(void (*function)(void*), void* userdata = nullptr,
		                          ThreadPriority priority = ThreadPriority::Normal)
		{
			return create("Trinex Thread", function, userdata, priority);
		}
	};
}// namespace Trinex::Platform
