#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <Core/structures.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanSampler;
	class VulkanSRV;
	class VulkanUAV;
	struct VulkanDescriptorPool;

	struct VulkanDescriptorSetLayout : public VulkanDeferredDestroy<RHI_Object> {
		vk::DescriptorSetLayout layout = {};

		byte uniform_buffers        = 0;
		byte textures               = 0;
		byte samplers               = 0;
		byte combined_image_sampler = 0;

		FORCE_INLINE bool has_layouts() const { return static_cast<bool>(layout); }
		~VulkanDescriptorSetLayout();
	};


	struct VulkanDescriptorSet {
		VulkanDescriptorPool* pool       = nullptr;
		vk::DescriptorSet descriptor_set = {};

		VulkanDescriptorSet();
		VulkanDescriptorSet& bind(vk::PipelineLayout& layout, vk::PipelineBindPoint point = vk::PipelineBindPoint::eGraphics);

		VulkanDescriptorSet& bind_uniform_buffer(const vk::DescriptorBufferInfo& info, BindLocation location,
		                                         vk::DescriptorType type);
		VulkanDescriptorSet& bind_sampler(VulkanSampler* sampler, BindLocation location);
		VulkanDescriptorSet& bind_srv(VulkanSRV* srv, byte location, VulkanSampler* sampler);
		VulkanDescriptorSet& bind_uav(VulkanUAV* uav, byte location);
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
