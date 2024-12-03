#pragma once
#include <Core/etl/map.hpp>
#include <Core/structures.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanSampler;
	struct VulkanTexture;
	struct VulkanSSBO;
	struct VulkanDescriptorPool;
	struct VulkanDescriptorSetLayout;

	struct VulkanDescriptorSet {
		VulkanDescriptorPool* pool       = nullptr;
		vk::DescriptorSet descriptor_set = {};

		VulkanDescriptorSet();
		VulkanDescriptorSet& bind(vk::PipelineLayout& layout, vk::PipelineBindPoint point = vk::PipelineBindPoint::eGraphics);

		VulkanDescriptorSet& bind_ssbo(struct VulkanSSBO* ssbo, BindLocation location);
		VulkanDescriptorSet& bind_uniform_buffer(const vk::DescriptorBufferInfo& info, BindLocation location,
		                                         vk::DescriptorType type);
		VulkanDescriptorSet& bind_sampler(VulkanSampler* sampler, BindLocation location);
		VulkanDescriptorSet& bind_texture(VulkanTexture* texture, BindLocation location);
		VulkanDescriptorSet& bind_texture_combined(VulkanTexture*, VulkanSampler*, BindLocation location);
	};

	struct VulkanDescriptorSetList {
		Vector<VulkanDescriptorSet*> m_sets;
		size_t m_next  = 0;
		short_t m_wait = VK_DESCRIPTOR_WAIT_FRAMES;


		VulkanDescriptorSet* alloc();
		VulkanDescriptorSetList& add(VulkanDescriptorSet* set);
		bool reset();
		VulkanDescriptorSetList& destroy(VulkanDescriptorSetLayout* layout);
	};

	struct VulkanDescriptorSetManager {
	private:
		Vector<VulkanDescriptorPool*> m_descriptor_pools;
		Map<VulkanDescriptorSetLayout*, struct VulkanDescriptorSetList> m_descriptor_set_lists;

		VulkanDescriptorSetList* find_list(VulkanDescriptorSetLayout* layout);

	public:
		VulkanDescriptorSet* allocate_descriptor_set(VulkanDescriptorSetLayout* layout);
		VulkanDescriptorSetManager& submit();
		~VulkanDescriptorSetManager();
	};
}// namespace Engine
