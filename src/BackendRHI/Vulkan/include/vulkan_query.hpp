#pragma once
#include <Core/etl/vector.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanQueryPool
	{
	private:
		vk::QueryPool m_pool;
		Vector<u64> m_free;
		u64 m_index = 0;

	public:
		VulkanQueryPool(const vk::QueryPoolCreateInfo& info);
		~VulkanQueryPool();

		bool find_index(u64& index);
		bool is_available(u64 index);
		bool query(u64 index, void* dst, usize size);
		VulkanQueryPool& release_index(u64 index);

		inline bool has_free_indices() const { return m_index < m_free.size() * 64 && m_free[m_index / 64] != 0; }
		inline vk::QueryPool pool() const { return m_pool; }
	};

	class VulkanQueryPoolManager
	{
	private:
		static constexpr vk::QueryPipelineStatisticFlags s_stats_flags =
		        vk::QueryPipelineStatisticFlagBits::eInputAssemblyVertices |
		        vk::QueryPipelineStatisticFlagBits::eInputAssemblyPrimitives |
		        vk::QueryPipelineStatisticFlagBits::eVertexShaderInvocations |
		        vk::QueryPipelineStatisticFlagBits::eGeometryShaderInvocations |
		        vk::QueryPipelineStatisticFlagBits::eGeometryShaderPrimitives |
		        vk::QueryPipelineStatisticFlagBits::eClippingInvocations |
		        vk::QueryPipelineStatisticFlagBits::eClippingPrimitives |
		        vk::QueryPipelineStatisticFlagBits::eFragmentShaderInvocations |
		        vk::QueryPipelineStatisticFlagBits::eTessellationEvaluationShaderInvocations |
		        vk::QueryPipelineStatisticFlagBits::eTessellationControlShaderPatches;

		Vector<VulkanQueryPool*> m_timestamp_pools;
		Vector<VulkanQueryPool*> m_occlusion_pools;
		Vector<VulkanQueryPool*> m_statistics_pools;

		vk::QueryPoolCreateInfo m_timestamp_info;
		vk::QueryPoolCreateInfo m_occlustion_info;
		vk::QueryPoolCreateInfo m_statistics_info;

		static VulkanQueryPool* find_pool(Vector<VulkanQueryPool*>& pool, const vk::QueryPoolCreateInfo& info);

	public:
		VulkanQueryPoolManager();
		~VulkanQueryPoolManager();

		inline VulkanQueryPool* find_timestamp_pool() { return find_pool(m_timestamp_pools, m_timestamp_info); }
		inline VulkanQueryPool* find_occlusion_pool() { return find_pool(m_occlusion_pools, m_occlustion_info); }
		inline VulkanQueryPool* find_statistics_pool() { return find_pool(m_statistics_pools, m_statistics_info); }
	};
}// namespace Engine
