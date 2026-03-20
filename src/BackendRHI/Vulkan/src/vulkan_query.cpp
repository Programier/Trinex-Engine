#include <vulkan_api.hpp>
#include <vulkan_context.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_query.hpp>
#include <vulkan_sync.hpp>

namespace Trinex
{
	VulkanQueryPool::VulkanQueryPool(const vk::QueryPoolCreateInfo& info)
	{
		m_pool = vk::check_result(API->m_device.createQueryPool(info));
		m_free.resize(info.queryCount / 64, ~static_cast<u64>(0));
	}

	VulkanQueryPool::~VulkanQueryPool()
	{
		API->m_device.destroyQueryPool(m_pool);
	}

	bool VulkanQueryPool::find_index(u64& index)
	{
		for (u32 word_index = m_index / 64, count = m_free.size(); word_index < count; ++word_index)
		{
			u64& word = m_free[word_index];

			if (word)
			{
				u64 bit_index = std::countr_zero(word);
				word &= ~(1 << bit_index);

				index   = bit_index + word_index * 64;
				m_index = index + 1;
				return true;
			}
		}
		return false;
	}

	bool VulkanQueryPool::is_available(u64 index)
	{
		struct {
			u64 value;
			u64 available;
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

	bool VulkanQueryPool::query(u64 index, void* dst, usize stride)
	{
		vk::Result res = API->m_device.getQueryPoolResults(m_pool, index, 1, stride, dst, stride, vk::QueryResultFlagBits::e64);
		return res == vk::Result::eSuccess;
	}

	VulkanQueryPool& VulkanQueryPool::release_index(u64 index)
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
			u64 index             = 0;

			inline bool is_valid() const { return pool != nullptr; }

			inline u64 query()
			{
				if (pool)
				{
					u64 result = 0;
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

				auto cmd = ctx->handle();
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
				u64 delta = m_end.query() - m_begin.query();
				return static_cast<float>(delta) * API->m_properties.limits.timestampPeriod / 1000000.0f;
			}
			return 0.f;
		}
	};

	class VulkanPipelineStats : public VulkanDeferredDestroy<RHIPipelineStatistics>
	{
	private:
		struct Stats {
			u64 vertices;
			u64 primitives;
			u64 vertex_shader_invocations;
			u64 geometry_shader_invocations;
			u64 geometry_shader_primitives;
			u64 clipping_invocations;
			u64 clipping_primitives;
			u64 fragment_shader_invocations;
			u64 tessellation_control_shader_invocations;
			u64 tesselation_shader_invocations;
		};

		struct Marker {
			VulkanQueryPool* pool = nullptr;
			u64 index             = 0;

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

				auto cmd = ctx->handle();
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
					auto cmd = ctx->handle();
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

}// namespace Trinex
