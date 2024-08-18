#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_descriptor_pool.hpp>
#include <vulkan_descriptor_set.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{
	VulkanDescriptorSet::VulkanDescriptorSet()
	{}

	VulkanDescriptorSet& VulkanDescriptorSet::bind(vk::PipelineLayout& layout, vk::PipelineBindPoint point)
	{
		auto cmd = API->current_command_buffer();
		cmd->m_cmd.bindDescriptorSets(point, layout, 0, descriptor_set, {});
		cmd->add_object(this);
		return *this;
	}

	VulkanDescriptorSet& VulkanDescriptorSet::bind_ssbo(struct VulkanSSBO* ssbo, BindLocation location)
	{
		vk::DescriptorBufferInfo buffer_info(ssbo->m_buffer.m_buffer, 0, ssbo->m_buffer.m_size);
		vk::WriteDescriptorSet write_descriptor(descriptor_set, location.binding, 0, vk::DescriptorType::eStorageBuffer,
												{}, buffer_info);
		API->m_device.updateDescriptorSets(write_descriptor, {});
		API->current_command_buffer()->add_object(ssbo);
		return *this;
	}

	VulkanDescriptorSet& VulkanDescriptorSet::bind_uniform_buffer(const vk::DescriptorBufferInfo& info,
																  BindLocation location, vk::DescriptorType type)
	{
		vk::WriteDescriptorSet write_descriptor(descriptor_set, location.binding, 0, type, {}, info);
		API->m_device.updateDescriptorSets(write_descriptor, {});
		return *this;
	}

	VulkanDescriptorSet& VulkanDescriptorSet::bind_sampler(VulkanSampler* sampler, BindLocation location)
	{
		vk::DescriptorImageInfo image_info(sampler->m_sampler, {}, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet write_descriptor(descriptor_set, location.binding, 0, vk::DescriptorType::eSampler,
												image_info);
		API->m_device.updateDescriptorSets(write_descriptor, {});
		API->current_command_buffer()->add_object(sampler);
		return *this;
	}

	VulkanDescriptorSet& VulkanDescriptorSet::bind_texture(VulkanTexture* texture, BindLocation location)
	{
		vk::DescriptorImageInfo image_info({}, texture->image_view(), vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet write_descriptor(descriptor_set, location.binding, 0, vk::DescriptorType::eSampledImage,
												image_info);
		API->m_device.updateDescriptorSets(write_descriptor, {});
		API->current_command_buffer()->add_object(texture);
		return *this;
	}

	VulkanDescriptorSet& VulkanDescriptorSet::bind_texture_combined(VulkanTexture* texture, VulkanSampler* sampler,
																	BindLocation location)
	{
		vk::DescriptorImageInfo image_info(sampler->m_sampler, texture->image_view(),
										   vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet write_descriptor(descriptor_set, location.binding, 0,
												vk::DescriptorType::eCombinedImageSampler, image_info);
		API->m_device.updateDescriptorSets(write_descriptor, {});
		API->current_command_buffer()->add_object(texture);
		API->current_command_buffer()->add_object(sampler);
		return *this;
	}
}// namespace Engine
