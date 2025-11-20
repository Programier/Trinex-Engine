#include <vulkan_api.hpp>
#include <vulkan_context.hpp>
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
		struct Marker {
			VulkanQueryPool* pool = nullptr;
			uint64_t index        = 0;

			inline bool is_valid() const { return pool != nullptr; }

			inline uint64_t query()
			{
				if (pool)
				{
					uint64_t result = 0;
					pool->query(index, &result, sizeof(result));
					return result;
				}

				return 0.f;
			}

			void release()
			{
				if (pool)
				{
					pool->release_index(index);
					pool  = nullptr;
					index = 0;
				}
			}

			~Marker() { release(); }
		};

		Marker m_begin;
		Marker m_end;

		static void write_timestamp(Marker& marker, VulkanQueryPoolManager* manager, VulkanContext* ctx)
		{
			if (API->m_properties.limits.timestampComputeAndGraphics)
			{
				if (!marker.is_valid())
				{
					marker.pool = manager->find_timestamp_pool();
					marker.pool->find_index(marker.index);
				}

				auto cmd = ctx->end_render_pass();
				cmd->resetQueryPool(marker.pool->pool(), marker.index, 1);
				cmd->writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe, marker.pool->pool(), marker.index);
			}
		}

	public:
		VulkanTimestamp& begin(VulkanQueryPoolManager* manager, VulkanContext* ctx)
		{
			write_timestamp(m_begin, manager, ctx);
			return *this;
		}

		VulkanTimestamp& end(VulkanQueryPoolManager* manager, VulkanContext* ctx)
		{
			write_timestamp(m_end, manager, ctx);
			return *this;
		}

		float milliseconds() override
		{
			if (m_begin.is_valid() && m_end.is_valid())
			{
				uint64_t delta = m_end.query() - m_begin.query();
				return static_cast<float>(delta) * API->m_properties.limits.timestampPeriod / 1000000.0f;
			}
			return 0.f;
		}
	};

	class VulkanPipelineStats : public VulkanDeferredDestroy<RHIPipelineStatistics>
	{
	private:
		struct Stats {
			uint64_t vertices;
			uint64_t primitives;
			uint64_t vertex_shader_invocations;
			uint64_t geometry_shader_invocations;
			uint64_t geometry_shader_primitives;
			uint64_t clipping_invocations;
			uint64_t clipping_primitives;
			uint64_t fragment_shader_invocations;
			uint64_t tessellation_control_shader_invocations;
			uint64_t tesselation_shader_invocations;
		};

		struct Marker {
			VulkanQueryPool* pool = nullptr;
			uint64_t index        = 0;

			inline bool is_valid() const { return pool != nullptr; }

			inline void query(Stats* stats)
			{
				if (pool)
				{
					pool->query(index, stats, sizeof(Stats));

					pool->release_index(index);
					pool  = nullptr;
					index = 0;
				}
			}
		};

		Marker m_marker;

	public:
		VulkanPipelineStats& begin(VulkanQueryPoolManager* manager, VulkanContext* ctx)
		{
			if (API->m_features.pipelineStatisticsQuery)
			{
				if (!m_marker.is_valid())
				{
					m_marker.pool = manager->find_statistics_pool();
					m_marker.pool->find_index(m_marker.index);
				}

				auto cmd = ctx->end_render_pass();
				cmd->resetQueryPool(m_marker.pool->pool(), m_marker.index, 1);
				cmd->beginQuery(m_marker.pool->pool(), m_marker.index, {});
			}
			return *this;
		}

		VulkanPipelineStats& end(VulkanQueryPoolManager* manager, VulkanContext* ctx)
		{
			if (API->m_features.pipelineStatisticsQuery)
			{
				if (m_marker.is_valid())
				{
					auto cmd = ctx->end_render_pass();
					cmd->endQuery(m_marker.pool->pool(), m_marker.index);
				}
			}

			return *this;
		}

		VulkanPipelineStats& fetch()
		{
			Stats stats;
			m_marker.query(&stats);

			vertices                                = stats.vertices;
			primitives                              = stats.primitives;
			vertex_shader_invocations               = stats.vertex_shader_invocations;
			geometry_shader_invocations             = stats.geometry_shader_invocations;
			geometry_shader_primitives              = stats.geometry_shader_primitives;
			clipping_invocations                    = stats.clipping_invocations;
			clipping_primitives                     = stats.clipping_primitives;
			fragment_shader_invocations             = stats.fragment_shader_invocations;
			tessellation_control_shader_invocations = stats.tessellation_control_shader_invocations;
			tesselation_shader_invocations          = stats.tesselation_shader_invocations;
			return *this;
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

	VulkanContext& VulkanContext::begin_timestamp(RHITimestamp* timestamp)
	{
		auto manager = API->query_pool_manager();
		static_cast<VulkanTimestamp*>(timestamp)->begin(manager, this);
		return *this;
	}

	VulkanContext& VulkanContext::end_timestamp(RHITimestamp* timestamp)
	{
		auto manager = API->query_pool_manager();
		static_cast<VulkanTimestamp*>(timestamp)->end(manager, this);
		return *this;
	}

	VulkanContext& VulkanContext::begin_statistics(RHIPipelineStatistics* stats)
	{
		auto manager = API->query_pool_manager();
		static_cast<VulkanPipelineStats*>(stats)->begin(manager, this);
		return *this;
	}

	VulkanContext& VulkanContext::end_statistics(RHIPipelineStatistics* stats)
	{
		auto manager = API->query_pool_manager();
		static_cast<VulkanPipelineStats*>(stats)->end(manager, this);
		return *this;
	}

}// namespace Engine
