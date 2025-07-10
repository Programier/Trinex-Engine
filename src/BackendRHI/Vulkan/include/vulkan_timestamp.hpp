#pragma once
#include <Core/etl/vector.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanQueryPool
	{
	private:
		vk::QueryPool m_pool;
		Vector<uint64_t> m_free;
		uint64_t m_index = 0;

	public:
		VulkanQueryPool(const vk::QueryPoolCreateInfo& info);
		~VulkanQueryPool();

		bool find_index(uint64_t& index);
		bool is_available(uint64_t index);
		uint64_t query(uint64_t index);
		VulkanQueryPool& release_index(uint64_t index);

		inline bool has_free_indices() const { return m_index < m_free.size() * 64 && m_free[m_index / 64] != 0; }
		inline vk::QueryPool pool() const { return m_pool; }
	};

	class VulkanQueryPoolManager
	{
	private:
		Vector<VulkanQueryPool*> m_timestamp_pools;
		Vector<VulkanQueryPool*> m_occlusion_pools;
		vk::QueryPoolCreateInfo m_timestamp_info;
		vk::QueryPoolCreateInfo m_occlustion_info;

		static VulkanQueryPool* find_pool(Vector<VulkanQueryPool*>& pool, const vk::QueryPoolCreateInfo& info);

	public:
		VulkanQueryPoolManager();
		~VulkanQueryPoolManager();

		inline VulkanQueryPool* find_timestamp_pool() { return find_pool(m_timestamp_pools, m_timestamp_info); }
		VulkanQueryPool* find_occlusion_pool() { return find_pool(m_occlusion_pools, m_occlustion_info); }
	};
}// namespace Engine
