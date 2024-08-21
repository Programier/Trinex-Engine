#include <Core/exception.hpp>
#include <vulkan_api.hpp>
#include <vulkan_descript_set_layout.hpp>
#include <vulkan_descriptor_pool.hpp>
#include <vulkan_descriptor_set.hpp>
#include <vulkan_pipeline.hpp>

namespace Engine
{
	static constexpr inline uint32_t max_sets = 49152;

	void VulkanDescriptorPool::init()
	{
		info_log("Vulkan", "Allocate new descriptor pool!");
		free_sets              = max_sets;
		samplers               = max_sets;
		textures               = max_sets / 2;
		combined_image_sampler = max_sets / 4;
		uniform_buffers        = max_sets * 2;

		std::array<vk::DescriptorPoolSize, 4> pools = {{
		        {vk::DescriptorType::eSampler, samplers},
		        {vk::DescriptorType::eCombinedImageSampler, combined_image_sampler},
		        {vk::DescriptorType::eSampledImage, textures},
		        {vk::DescriptorType::eUniformBuffer, uniform_buffers},
		}};

		vk::DescriptorPoolCreateInfo info(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, max_sets, pools);
		pool = API->m_device.createDescriptorPool(info);
	}

	bool VulkanDescriptorPool::can_allocate(VulkanDescriptorSetLayout* layout)
	{
		return free_sets > 0 && samplers >= layout->samplers && textures >= layout->textures &&
		       combined_image_sampler >= layout->combined_image_sampler && uniform_buffers >= layout->uniform_buffers;
	}

	VulkanDescriptorSet* VulkanDescriptorPool::allocate_descriptor_set(VulkanDescriptorSetLayout* layout)
	{
		if (!can_allocate(layout))
			return nullptr;

		VulkanDescriptorSet* new_set = new VulkanDescriptorSet();
		new_set->pool                = this;
		vk::DescriptorSetAllocateInfo info(pool, layout->layout);
		new_set->descriptor_set = API->m_device.allocateDescriptorSets(info)[0];

		free_sets -= 1;
		samplers -= layout->samplers;
		textures -= layout->textures;
		combined_image_sampler -= layout->combined_image_sampler;
		uniform_buffers -= layout->uniform_buffers;

		return new_set;
	}

	VulkanDescriptorPool& VulkanDescriptorPool::release_descriptor_set(VulkanDescriptorSet* descriptor_set,
	                                                                   VulkanDescriptorSetLayout* layout)
	{
		free_sets += 1;
		samplers += layout->samplers;
		textures += layout->textures;
		combined_image_sampler += layout->combined_image_sampler;
		uniform_buffers += layout->uniform_buffers;

		API->m_device.freeDescriptorSets(pool, descriptor_set->descriptor_set);
		descriptor_set->release();
		return *this;
	}

	VulkanDescriptorPool::~VulkanDescriptorPool()
	{
		DESTROY_CALL(destroyDescriptorPool, pool);
	}

	namespace VulkanDescriptorPoolManager
	{
		static Vector<VulkanDescriptorPool*> m_descriptor_pools;

		VulkanDescriptorSet* allocate_descriptor_set(VulkanDescriptorSetLayout* layout)
		{
			for (auto& pool : m_descriptor_pools)
			{
				if (VulkanDescriptorSet* set = pool->allocate_descriptor_set(layout))
				{
					return set;
				}
			}

			VulkanDescriptorPool* new_pool = new VulkanDescriptorPool();
			new_pool->init();
			m_descriptor_pools.push_back(new_pool);

			VulkanDescriptorSet* new_set = new_pool->allocate_descriptor_set(layout);
			if (new_set == nullptr)
				throw EngineException("Failed to allocate new descriptor set!");
			return new_set;
		}

		void release_descriptor_set(VulkanDescriptorSet* set, VulkanDescriptorSetLayout* layout)
		{
			set->pool->release_descriptor_set(set, layout);
		}

		void release_all()
		{
			for (auto& pool : m_descriptor_pools)
			{
				trinex_always_check(pool->free_sets == max_sets,
				                    "All descriptor sets must be released before destroying descriptor pool!");
				delete pool;
			}
			m_descriptor_pools.clear();
		}
	}// namespace VulkanDescriptorPoolManager
}// namespace Engine
