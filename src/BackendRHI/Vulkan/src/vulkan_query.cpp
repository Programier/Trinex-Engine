#include <vulkan_api.hpp>
#include <vulkan_commands.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_fence.hpp>
#include <vulkan_query.hpp>

namespace Engine
{
	VulkanQueryPool::VulkanQueryPool(const vk::QueryPoolCreateInfo& info)
	{
		m_pool = API->m_device.createQueryPool(info);
		m_free.resize(info.queryCount / 64, ~static_cast<uint64_t>(0));
	}

	VulkanQueryPool::~VulkanQueryPool()
	{
		API->m_device.destroyQueryPool(m_pool);
	}

	bool VulkanQueryPool::find_index(uint64_t& index)
	{
		for (uint32_t word_index = m_index / 64, count = m_free.size(); word_index < count; ++word_index)
		{
			uint64_t& word = m_free[word_index];

			if (word)
			{
				uint64_t bit_index = std::countr_zero(word);
				word &= ~(1 << bit_index);

				index   = bit_index + word_index * 64;
				m_index = index + 1;
				return true;
			}
		}
		return false;
	}

	bool VulkanQueryPool::is_available(uint64_t index)
	{
		struct {
			uint64_t value;
			uint64_t available;
		} result{};

		vk::Result res =
		        API->m_device.getQueryPoolResults(m_pool, index, 1, sizeof(result), &result, sizeof(result),
		                                          vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWithAvailability);

		if (res == vk::Result::eSuccess)
		{
			return result.available != 0;
		}

		return false;
	}

	bool VulkanQueryPool::query(uint64_t index, void* dst, size_t stride)
	{
		vk::Result res = API->m_device.getQueryPoolResults(m_pool, index, 1, stride, dst, stride, vk::QueryResultFlagBits::e64);
		return res == vk::Result::eSuccess;
	}

	VulkanQueryPool& VulkanQueryPool::release_index(uint64_t index)
	{
		if (m_index > index)
			m_index = index;

		m_free[index / 64] |= (1 << (index % 64));
		return *this;
	}

	VulkanQueryPoolManager::VulkanQueryPoolManager()
	    : m_timestamp_info({}, vk::QueryType::eTimestamp, 1024),
	      m_statistics_info({}, vk::QueryType::ePipelineStatistics, 1024, s_stats_flags)
	{}

	VulkanQueryPoolManager::~VulkanQueryPoolManager()
	{
		auto destroy_pool = [](Vector<VulkanQueryPool*>& pools) {
			for (VulkanQueryPool* pool : pools)
			{
				trx_delete pool;
			}
		};

		destroy_pool(m_occlusion_pools);
		destroy_pool(m_timestamp_pools);
		destroy_pool(m_statistics_pools);
	}

	VulkanQueryPool* VulkanQueryPoolManager::find_pool(Vector<VulkanQueryPool*>& pools, const vk::QueryPoolCreateInfo& info)
	{
		for (VulkanQueryPool* pool : pools)
		{
			if (pool->has_free_indices())
				return pool;
		}

		VulkanQueryPool* pool = trx_new VulkanQueryPool(info);
		pools.push_back(pool);
		return pool;
	}

	class VulkanTimestamp : public VulkanDeferredDestroy<RHITimestamp>
	{
	private:
		float m_last_result = 0.f;

		struct Marker {
			VulkanQueryPool* pool = nullptr;
			uint64_t index        = 0;
			VulkanFenceRef fence;

			inline bool is_valid() const { return pool != nullptr; }
			inline bool is_ready() { return fence.is_signaled() || !fence.is_waiting(); }

			inline uint64_t query()
			{
				if (pool)
				{
					uint64_t result = 0;
					pool->query(index, &result, sizeof(result));

					pool->release_index(index);
					pool  = nullptr;
					index = 0;

					return result;
				}
				return 0;
			}
		};

		Marker m_begin;
		Marker m_end;

		static void write_timestamp(Marker& marker, VulkanQueryPoolManager* manager, VulkanCommandHandle* cmd)
		{
			if (API->m_properties.limits.timestampComputeAndGraphics)
			{
				if (!marker.is_valid())
				{
					marker.pool = manager->find_timestamp_pool();
					marker.pool->find_index(marker.index);
				}

				API->end_render_pass();
				cmd->resetQueryPool(marker.pool->pool(), marker.index, 1);
				cmd->writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe, marker.pool->pool(), marker.index);
				marker.fence.signal(cmd);
			}
		}

	public:
		VulkanTimestamp& begin(VulkanQueryPoolManager* manager, VulkanCommandHandle* cmd)
		{
			write_timestamp(m_begin, manager, cmd);
			return *this;
		}

		VulkanTimestamp& end(VulkanQueryPoolManager* manager, VulkanCommandHandle* cmd)
		{
			write_timestamp(m_end, manager, cmd);
			return *this;
		}

		bool is_ready() override
		{
			if (!m_begin.is_valid())
				return true;

			// "Begin" is signaled, but "end" is not signaled, so, timestamp is in waiting state now
			if (!m_end.is_valid())
				return false;

			bool ready = m_begin.is_ready() && m_end.is_ready();

			if (ready)
			{
				uint64_t begin = m_begin.query();
				uint64_t end   = m_end.query();

				float timestamp_period_ns = API->m_properties.limits.timestampPeriod;
				m_last_result             = static_cast<float>((end - begin) * timestamp_period_ns * 1e-6);

				return m_last_result;
			}
			return ready;
		}

		float milliseconds() override
		{
			if (is_ready())
				return m_last_result;
			return 0.f;
		}
	};

	class VulkanPipelineStats : public VulkanDeferredDestroy<RHIPipelineStatistics>
	{
	private:
		struct Stats {
			uint64_t m_vertices                                = 0;
			uint64_t m_primitives                              = 0;
			uint64_t m_vertex_shader_invocations               = 0;
			uint64_t m_geometry_shader_invocations             = 0;
			uint64_t m_geometry_shader_primitives              = 0;
			uint64_t m_clipping_invocations                    = 0;
			uint64_t m_clipping_primitives                     = 0;
			uint64_t m_fragment_shader_invocations             = 0;
			uint64_t m_tessellation_control_shader_invocations = 0;
			uint64_t m_tesselation_shader_invocations          = 0;
		};

		struct Marker {
			VulkanQueryPool* pool = nullptr;
			uint64_t index        = 0;
			VulkanFenceRef fence;

			inline bool is_valid() const { return pool != nullptr; }
			inline bool is_ready() { return fence.is_signaled(); }

			inline void query(Stats& stats)
			{
				if (pool)
				{
					pool->query(index, &stats, sizeof(stats));

					pool->release_index(index);
					pool  = nullptr;
					index = 0;
					fence.reset();
				}
			}
		};

		Stats m_stats;
		Marker m_marker;

	public:
		VulkanPipelineStats& begin(VulkanQueryPoolManager* manager, VulkanCommandHandle* cmd)
		{
			if (API->m_features.pipelineStatisticsQuery)
			{
				if (!m_marker.is_valid())
				{
					m_marker.pool = manager->find_statistics_pool();
					m_marker.pool->find_index(m_marker.index);
				}

				API->end_render_pass();
				cmd->resetQueryPool(m_marker.pool->pool(), m_marker.index, 1);
				cmd->beginQuery(m_marker.pool->pool(), m_marker.index, {});
				m_marker.fence.reset();
			}
			return *this;
		}

		VulkanPipelineStats& end(VulkanQueryPoolManager* manager, VulkanCommandHandle* cmd)
		{
			if (API->m_features.pipelineStatisticsQuery)
			{
				if (m_marker.is_valid())
				{
					API->end_render_pass();
					cmd->endQuery(m_marker.pool->pool(), m_marker.index);
					m_marker.fence.signal(cmd);
				}
				else
				{
					m_stats = {};
				}
			}

			return *this;
		}

		bool is_ready() override
		{
			if (!m_marker.is_valid())
				return true;

			if (m_marker.is_ready())
			{
				m_marker.query(m_stats);
				return true;
			}

			return false;
		}

		const Stats& query_stats()
		{
			if (is_ready())
			{
				return m_stats;
			}

			static const Stats default_stats;
			return default_stats;
		}

		uint64_t vertices() override { return query_stats().m_vertices; }
		uint64_t primitives() override { return query_stats().m_primitives; }
		uint64_t geometry_shader_primitives() override { return query_stats().m_geometry_shader_primitives; }
		uint64_t clipping_primitives() override { return query_stats().m_clipping_primitives; }
		uint64_t vertex_shader_invocations() override { return query_stats().m_vertex_shader_invocations; }
		uint64_t tesselation_shader_invocations() override { return query_stats().m_tesselation_shader_invocations; }
		uint64_t geometry_shader_invocations() override { return query_stats().m_geometry_shader_invocations; }
		uint64_t clipping_invocations() override { return query_stats().m_clipping_invocations; }
		uint64_t fragment_shader_invocations() override { return query_stats().m_fragment_shader_invocations; }

		uint64_t tessellation_control_shader_invocations() override
		{
			return query_stats().m_tessellation_control_shader_invocations;
		}
	};

	RHITimestamp* VulkanAPI::create_timestamp()
	{
		return trx_new VulkanTimestamp();
	}

	RHIPipelineStatistics* VulkanAPI::create_pipeline_statistics()
	{
		return trx_new VulkanPipelineStats();
	}

	VulkanAPI& VulkanAPI::begin_timestamp(RHITimestamp* timestamp)
	{
		static_cast<VulkanTimestamp*>(timestamp)->begin(m_query_pool_manager, current_command_buffer());
		return *this;
	}

	VulkanAPI& VulkanAPI::end_timestamp(RHITimestamp* timestamp)
	{
		static_cast<VulkanTimestamp*>(timestamp)->end(m_query_pool_manager, current_command_buffer());
		return *this;
	}

	VulkanAPI& VulkanAPI::begin_statistics(RHIPipelineStatistics* stats)
	{
		static_cast<VulkanPipelineStats*>(stats)->begin(m_query_pool_manager, current_command_buffer());
		return *this;
	}

	VulkanAPI& VulkanAPI::end_statistics(RHIPipelineStatistics* stats)
	{
		static_cast<VulkanPipelineStats*>(stats)->end(m_query_pool_manager, current_command_buffer());
		return *this;
	}

}// namespace Engine
