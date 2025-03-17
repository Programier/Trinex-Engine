#include <Core/exception.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_descript_set_layout.hpp>
#include <vulkan_descriptor_set.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{
	static constexpr inline uint32_t max_sets = 49152;

	VulkanDescriptorSet::VulkanDescriptorSet() {}

	VulkanDescriptorSet& VulkanDescriptorSet::bind(vk::PipelineLayout& layout, vk::PipelineBindPoint point)
	{
		auto cmd = API->current_command_buffer();
		cmd->m_cmd.bindDescriptorSets(point, layout, 0, descriptor_set, {});
		return *this;
	}

	VulkanDescriptorSet& VulkanDescriptorSet::bind_ssbo(struct VulkanSSBO* ssbo, BindLocation location)
	{
		vk::DescriptorBufferInfo buffer_info(ssbo->m_buffer.m_buffer, 0, ssbo->m_buffer.m_size);
		vk::WriteDescriptorSet write_descriptor(descriptor_set, location.binding, 0, vk::DescriptorType::eStorageBuffer, {},
		                                        buffer_info);
		API->m_device.updateDescriptorSets(write_descriptor, {});
		API->current_command_buffer()->add_object(ssbo);
		return *this;
	}

	VulkanDescriptorSet& VulkanDescriptorSet::bind_uniform_buffer(const vk::DescriptorBufferInfo& info, BindLocation location,
	                                                              vk::DescriptorType type)
	{
		vk::WriteDescriptorSet write_descriptor(descriptor_set, location.binding, 0, type, {}, info);
		API->m_device.updateDescriptorSets(write_descriptor, {});
		return *this;
	}

	VulkanDescriptorSet& VulkanDescriptorSet::bind_sampler(VulkanSampler* sampler, BindLocation location)
	{
		vk::DescriptorImageInfo image_info(sampler->m_sampler, {}, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet write_descriptor(descriptor_set, location.binding, 0, vk::DescriptorType::eSampler, image_info);
		API->m_device.updateDescriptorSets(write_descriptor, {});
		API->current_command_buffer()->add_object(sampler);
		return *this;
	}

	VulkanDescriptorSet& VulkanDescriptorSet::bind_texture(VulkanTextureSRV* texture, BindLocation location)
	{
		vk::DescriptorImageInfo image_info({}, texture->m_view, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet write_descriptor(descriptor_set, location.binding, 0, vk::DescriptorType::eSampledImage,
		                                        image_info);
		API->m_device.updateDescriptorSets(write_descriptor, {});
		API->current_command_buffer()->add_object(texture);
		return *this;
	}

	VulkanDescriptorSet& VulkanDescriptorSet::bind_texture(VulkanTextureUAV* texture, BindLocation location)
	{
		vk::DescriptorImageInfo image_info({}, texture->m_view, vk::ImageLayout::eGeneral);
		vk::WriteDescriptorSet write_descriptor(descriptor_set, location.binding, 0, vk::DescriptorType::eStorageImage,
												image_info);
		API->m_device.updateDescriptorSets(write_descriptor, {});
		API->current_command_buffer()->add_object(texture);
		return *this;
	}

	VulkanDescriptorSet& VulkanDescriptorSet::bind_texture_combined(VulkanTextureSRV* texture, VulkanSampler* sampler,
	                                                                BindLocation location)
	{
		vk::DescriptorImageInfo image_info(sampler->m_sampler, texture->m_view, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet write_descriptor(descriptor_set, location.binding, 0, vk::DescriptorType::eCombinedImageSampler,
		                                        image_info);
		API->m_device.updateDescriptorSets(write_descriptor, {});
		API->current_command_buffer()->add_object(texture);
		API->current_command_buffer()->add_object(sampler);
		return *this;
	}


	struct VulkanDescriptorPool {
		vk::DescriptorPool pool;

		uint32_t free_sets              = 0;
		uint32_t samplers               = 0;
		uint32_t textures               = 0;
		uint32_t combined_image_sampler = 0;
		uint32_t uniform_buffers        = 0;

		VulkanDescriptorPool()
		{
			info_log("Vulkan", "Allocate new descriptor pool!");
			free_sets              = max_sets;
			samplers               = max_sets * 16;
			textures               = max_sets * 16;
			combined_image_sampler = max_sets * 16;
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

		bool can_allocate(VulkanDescriptorSetLayout* layout)
		{
			return free_sets > 0 && samplers >= layout->samplers && textures >= layout->textures &&
			       combined_image_sampler >= layout->combined_image_sampler && uniform_buffers >= layout->uniform_buffers;
		}

		VulkanDescriptorSet* allocate_descriptor_set(VulkanDescriptorSetLayout* layout)
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

		VulkanDescriptorPool& release_descriptor_set(VulkanDescriptorSet* descriptor_set, VulkanDescriptorSetLayout* layout)
		{
			free_sets += 1;
			samplers += layout->samplers;
			textures += layout->textures;
			combined_image_sampler += layout->combined_image_sampler;
			uniform_buffers += layout->uniform_buffers;

			API->m_device.freeDescriptorSets(pool, descriptor_set->descriptor_set);
			delete descriptor_set;
			return *this;
		}

		~VulkanDescriptorPool() { DESTROY_CALL(destroyDescriptorPool, pool); }
	};


	VulkanDescriptorSet* VulkanDescriptorSetList::alloc()
	{
		if (m_next == m_sets.size())
			return nullptr;

		return m_sets[m_next++];
	}

	VulkanDescriptorSetList& VulkanDescriptorSetList::add(VulkanDescriptorSet* set)
	{
		m_sets.push_back(set);
		m_next = m_sets.size();
		return *this;
	}

	bool VulkanDescriptorSetList::reset()
	{
		if (m_next == 0)
		{
			--m_wait;
		}
		else
		{
			m_wait = VK_DESCRIPTOR_WAIT_FRAMES;
			m_next = 0;
		}
		return m_wait == 0;
	}

	VulkanDescriptorSetList& VulkanDescriptorSetList::destroy(VulkanDescriptorSetLayout* layout)
	{
		for (auto set : m_sets)
		{
			set->pool->release_descriptor_set(set, layout);
		}
		m_sets.clear();
		return *this;
	}


	VulkanDescriptorSetList* VulkanDescriptorSetManager::find_list(VulkanDescriptorSetLayout* layout)
	{
		auto it = m_descriptor_set_lists.find(layout);

		if (it == m_descriptor_set_lists.end())
		{
			layout->add_reference();
			return &m_descriptor_set_lists[layout];
		}
		else
		{
			return &(it->second);
		}
	}

	VulkanDescriptorSet* VulkanDescriptorSetManager::allocate_descriptor_set(VulkanDescriptorSetLayout* layout)
	{
		auto* list = find_list(layout);

		if (VulkanDescriptorSet* set = list->alloc())
		{
			return set;
		}

		for (auto& pool : m_descriptor_pools)
		{
			if (VulkanDescriptorSet* set = pool->allocate_descriptor_set(layout))
			{
				list->add(set);
				return set;
			}
		}

		VulkanDescriptorPool* new_pool = new VulkanDescriptorPool();
		m_descriptor_pools.push_back(new_pool);

		VulkanDescriptorSet* new_set = new_pool->allocate_descriptor_set(layout);
		if (new_set == nullptr)
			throw EngineException("Failed to allocate new descriptor set!");

		list->add(new_set);
		return new_set;
	}

	VulkanDescriptorSetManager& VulkanDescriptorSetManager::submit()
	{
		auto begin = m_descriptor_set_lists.begin();

		while (begin != m_descriptor_set_lists.end())
		{
			auto& pair = *begin;

			if (pair.second.reset())
			{
				pair.second.destroy(pair.first);
				pair.first->release();

				begin = m_descriptor_set_lists.erase(begin);
			}
			else
			{
				++begin;
			}
		}
		return *this;
	}

	VulkanDescriptorSetManager::~VulkanDescriptorSetManager()
	{
		for (auto& [layout, info] : m_descriptor_set_lists)
		{
			info.destroy(layout);
			layout->release();
		}

		for (auto& pool : m_descriptor_pools)
		{
			delete pool;
		}
		m_descriptor_pools.clear();
	}
}// namespace Engine
