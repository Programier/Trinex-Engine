#pragma once
#include <Core/structures.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanSampler;
	struct VulkanTexture;
	struct VulkanSSBO;
	struct VulkanDescriptorPool;
	struct VulkanDescriptorSetLayout;

	struct VulkanDescriptorSet : public RHI_DefaultDestroyable<RHI_Object> {
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
}// namespace Engine
