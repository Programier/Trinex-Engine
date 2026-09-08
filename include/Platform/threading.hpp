#pragma once
#include <Core/etl/function.hpp>
#include <Core/etl/string.hpp>
#include <Platform/enums.hpp>
#include <Platform/object.hpp>
#include <Platform/types.hpp>

namespace Trinex::Platform
{
	using ThreadFunction = Function<i32()>;

	struct ThreadDesc {
		String name;
		ThreadFunction function;
		usize stack_size        = 0;
		ThreadPriority priority = ThreadPriority::Normal;
	};

	class ENGINE_EXPORT Thread : public Object
	{
	public:
		virtual ThreadId id() const                       = 0;
		virtual bool is_running() const                   = 0;
		virtual bool join(u64 nanoseconds)                = 0;
		virtual Thread* detach()                          = 0;
		virtual Thread* priority(ThreadPriority priority) = 0;
		virtual ThreadPriority priority() const           = 0;
	};

	class ENGINE_EXPORT ThreadSystem
	{
	public:
		static ThreadSystem* instance();

		virtual ~ThreadSystem() = default;

		virtual Thread* create_thread(const ThreadDesc* desc)      = 0;
		virtual ThreadId current_thread_id() const                 = 0;
		virtual ThreadSystem* current_thread_name(StringView name) = 0;
		virtual String current_thread_name() const                 = 0;
		virtual ThreadSystem* sleep(u64 nanoseconds)               = 0;
		virtual ThreadSystem* yield()                              = 0;
	};
}// namespace Trinex::Platform
