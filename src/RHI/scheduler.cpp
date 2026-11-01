#include <Graphics/render_pools.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>
#include <RHI/scheduler.hpp>

namespace Trinex
{
	struct RHISchedulerBatch
	{
		RHIQueueFlags queue = RHIQueueFlags::Graphics;
		Array<RHICommandHandle*> commands;
	};

	static void submit(Array<RHISchedulerBatch>& batches, usize& batch_count, RHICommandHandle* command, RHIQueueFlags queue)
	{
		trinex_assert(command);

		for (usize index = 0; index < batch_count; ++index)
		{
			auto& batch = batches[index];
			if (batch.queue == queue)
			{
				batch.commands.push_back(command);
				return;
			}
		}

		if (batch_count == batches.size())
			batches.emplace_back();

		auto& batch = batches[batch_count++];
		trinex_assert(batch.commands.empty());
		batch.queue = queue;
		batch.commands.push_back(command);
	}

	RHIScheduler* RHIScheduler::instance()
	{
		static RHIScheduler scheduler;
		return &scheduler;
	}

	void RHIScheduler::enqueue(RHIQueueFlags queue, Delegate<void(RHIContext*)> callback)
	{
		trinex_assert(callback);
		ScopeLock lock(m_pending_critical);
		m_pending.push_back({queue, etl::move(callback)});
	}

	void RHIScheduler::execute()
	{
		ScopeLock execution_lock(m_execution_critical);

		if (RHI::backend() == RHIBackend::Undefined)
			return;

		struct Recording
		{
			RHIQueueFlags queue;
			RHIContext* context;
		};

		static thread_local Array<Recording> recordings;
		static thread_local Array<Work> pending;
		static thread_local Array<RHISchedulerBatch> batches;
		usize batch_count = 0;

		if (recordings.capacity() == 0)
			recordings.reserve(4);

		if (batches.capacity() == 0)
			batches.reserve(4);

		auto* context_pool = RHIContextPool::instance();
		auto* rhi          = RHI::instance();

		{
			ScopeLock lock(m_pending_critical);
			pending.swap(m_pending);
		}

		for (Work& work : pending)
		{
			Recording* recording = nullptr;
			for (Recording& candidate : recordings)
			{
				if (candidate.queue == work.queue)
				{
					recording = &candidate;
					break;
				}
			}

			if (recording == nullptr)
			{
				recording = &recordings.emplace_back(Recording{work.queue, context_pool->begin()});
				trinex_verify(recording->context);
			}

			work.callback(recording->context);
		}

		for (Recording& recording : recordings)
		{
			if (RHICommandHandle* command = recording.context->end())
				submit(batches, batch_count, command, recording.queue);
			context_pool->release(recording.context);
		}

		for (usize index = 0; index < batch_count; ++index)
		{
			auto& batch     = batches[index];
			RHIQueue* queue = rhi->queue(batch.queue);
			trinex_verify(queue);

			RHISubmitInfo info = {.commands = {batch.commands.data(), batch.commands.size()}};
			queue->submit(info);

			for (RHICommandHandle* command : batch.commands)
				command->release();
			batch.commands.clear();
		}

		pending.clear();
		recordings.clear();
	}
} // namespace Trinex
