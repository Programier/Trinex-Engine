#pragma once
#include <vulkan_definitions.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanDescriptorSetLayout;
	struct VulkanDescriptorSet;

	struct VulkanDescriptorPool {
		vk::DescriptorPool pool;

		uint32_t free_sets				= 0;
		uint32_t samplers				= 0;
		uint32_t textures				= 0;
		uint32_t combined_image_sampler = 0;
		uint32_t uniform_buffers		= 0;

		void init();
		bool can_allocate(VulkanDescriptorSetLayout* layout);
		VulkanDescriptorSet* allocate_descriptor_set(VulkanDescriptorSetLayout* layout);
		VulkanDescriptorPool& release_descriptor_set(VulkanDescriptorSet*, VulkanDescriptorSetLayout* layout);


		~VulkanDescriptorPool();
	};


	namespace VulkanDescriptorPoolManager
	{
		VulkanDescriptorSet* allocate_descriptor_set(VulkanDescriptorSetLayout* layout);
		void release_descriptor_set(VulkanDescriptorSet*, VulkanDescriptorSetLayout* layout);
		void release_all();
	}// namespace VulkanDescriptorPoolManager
}// namespace Engine
