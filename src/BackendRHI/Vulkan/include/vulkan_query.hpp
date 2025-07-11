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
		bool query(uint64_t index, void* dst, size_t size);
		VulkanQueryPool& release_index(uint64_t index);

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
