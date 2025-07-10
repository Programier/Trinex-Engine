#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_timestamp.hpp>

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

	uint64_t VulkanQueryPool::query(uint64_t index)
	{
		uint64_t result = 0;
		vk::Result res  = API->m_device.getQueryPoolResults(m_pool, index, 1, sizeof(result), &result, sizeof(result),
		                                                    vk::QueryResultFlagBits::e64);
		if (res == vk::Result::eSuccess)
			return result;
		return 0;
	}

	VulkanQueryPool& VulkanQueryPool::release_index(uint64_t index)
	{
		if (m_index > index)
			m_index = index;

		m_free[index / 64] |= (1 << (index % 64));
		return *this;
	}

	VulkanQueryPoolManager::VulkanQueryPoolManager() : m_timestamp_info({}, vk::QueryType::eTimestamp, 1024) {}

	VulkanQueryPoolManager::~VulkanQueryPoolManager()
	{
		auto destroy_pool = [](Vector<VulkanQueryPool*>& pools) {
			for (VulkanQueryPool* pool : pools)
			{
				delete pool;
			}
		};

		destroy_pool(m_occlusion_pools);
		destroy_pool(m_timestamp_pools);
	}

	VulkanQueryPool* VulkanQueryPoolManager::find_pool(Vector<VulkanQueryPool*>& pools, const vk::QueryPoolCreateInfo& info)
	{
		for (VulkanQueryPool* pool : pools)
		{
			if (pool->has_free_indices())
				return pool;
		}

		VulkanQueryPool* pool = new VulkanQueryPool(info);
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

			inline bool is_valid() const { return pool != nullptr; }

			inline uint64_t query()
			{
				if (pool)
				{
					uint64_t result = pool->query(index);

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

		static void write_timestamp(Marker& marker, VulkanQueryPoolManager* manager, VulkanCommandBuffer* cmd)
		{
			if (API->m_properties.limits.timestampComputeAndGraphics)
			{
				if (!marker.is_valid())
				{
					marker.pool = manager->find_timestamp_pool();
					marker.pool->find_index(marker.index);
				}

				cmd->writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe, marker.pool->pool(), marker.index);
			}
		}

	public:
		VulkanTimestamp& begin(VulkanQueryPoolManager* manager, VulkanCommandBuffer* cmd)
		{
			write_timestamp(m_begin, manager, cmd);
			return *this;
		}

		VulkanTimestamp& end(VulkanQueryPoolManager* manager, VulkanCommandBuffer* cmd)
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

			bool ready = m_begin.pool->is_available(m_begin.index) && m_end.pool->is_available(m_end.index);

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

	RHITimestamp* VulkanAPI::create_timestamp()
	{
		return new VulkanTimestamp();
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

}// namespace Engine
