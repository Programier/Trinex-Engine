#pragma once

namespace Trinex
{
	struct Event;
	struct RawInputEventBatch;
}// namespace Trinex

namespace Trinex::Platform
{
	class ENGINE_EXPORT EventLoop
	{
	public:
		static EventLoop* instance();

		virtual ~EventLoop() = default;

		virtual EventLoop* poll_events()                     = 0;
		virtual EventLoop* wait_for_events()                 = 0;
		virtual EventLoop* wait_for_events(u32 timeout_ms)   = 0;
		virtual bool pump_raw_input(RawInputEventBatch* out) = 0;
		virtual EventLoop* wake()                            = 0;
	};
}// namespace Trinex::Platform
