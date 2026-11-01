#pragma once
#include <Core/etl/array.hpp>
#include <Core/etl/critical_section.hpp>
#include <Core/etl/delegate.hpp>
#include <RHI/enums.hpp>

namespace Trinex
{
	class RHIContext;
	class RHICommandHandle;
	class RHIContextPool;

	class trinex_api RHIScheduler final
	{
	private:
		struct Work
		{
			RHIQueueFlags queue = RHIQueueFlags::Graphics;
			Delegate<void(RHIContext*)> callback;
		};

		Array<Work> m_pending;
		CriticalSection m_pending_critical;
		CriticalSection m_execution_critical;

	public:
		static RHIScheduler* instance();

		// Records content work into one command buffer per queue on the next execute().
		void enqueue(RHIQueueFlags queue, Delegate<void(RHIContext*)> callback);
		void execute();
	};
} // namespace Trinex
