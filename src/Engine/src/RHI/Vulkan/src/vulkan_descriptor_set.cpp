#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_descriptor_set.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{
    VulkanDescriptorSet& VulkanDescriptorSet::init(vk::DescriptorPool& pool, vk::DescriptorSetLayout* layout)
    {
        vk::DescriptorSetAllocateInfo info(pool, *layout);
        m_set = API->m_device.allocateDescriptorSets(info).front();
        return *this;
    }

    VulkanDescriptorSet& VulkanDescriptorSet::bind(vk::PipelineLayout& layout, BindingIndex set, vk::PipelineBindPoint point,
                                                   const vk::ArrayProxy<uint32_t>& dynamic_offsets)
    {
        API->current_command_buffer().bindDescriptorSets(point, layout, set, m_set, dynamic_offsets);
        return *this;
    }

    VulkanDescriptorSet& VulkanDescriptorSet::bind_ssbo(struct VulkanSSBO* ssbo, BindingIndex index)
    {
        vk::DescriptorBufferInfo buffer_info(ssbo->m_buffer.m_buffer, 0, ssbo->m_buffer.m_size);
        vk::WriteDescriptorSet write_descriptor(m_set, index, 0, vk::DescriptorType::eStorageBuffer, {}, buffer_info);
        API->m_device.updateDescriptorSets(write_descriptor, {});
        return *this;
    }

    VulkanDescriptorSet& VulkanDescriptorSet::bind_uniform_buffer(const vk::DescriptorBufferInfo& info, BindingIndex index,
                                                                  vk::DescriptorType type)
    {
        vk::WriteDescriptorSet write_descriptor(m_set, index, 0, type, {}, info);
        API->m_device.updateDescriptorSets(write_descriptor, {});
        return *this;
    }

    VulkanDescriptorSet& VulkanDescriptorSet::bind_sampler(VulkanSampler* sampler, BindingIndex index)
    {
        vk::DescriptorImageInfo image_info(sampler->m_sampler, {}, vk::ImageLayout::eShaderReadOnlyOptimal);
        vk::WriteDescriptorSet write_descriptor(m_set, index, 0, vk::DescriptorType::eSampler, image_info);
        API->m_device.updateDescriptorSets(write_descriptor, {});
        return *this;
    }

    VulkanDescriptorSet& VulkanDescriptorSet::bind_texture(VulkanTexture* texture, BindingIndex index)
    {
        vk::DescriptorImageInfo image_info({}, texture->image_view(), vk::ImageLayout::eShaderReadOnlyOptimal);
        vk::WriteDescriptorSet write_descriptor(m_set, index, 0, vk::DescriptorType::eSampledImage, image_info);
        API->m_device.updateDescriptorSets(write_descriptor, {});
        return *this;
    }

    VulkanDescriptorSet& VulkanDescriptorSet::bind_texture_combined(VulkanTexture* texture, VulkanSampler* sampler,
                                                                    BindingIndex index)
    {
        vk::DescriptorImageInfo image_info(sampler->m_sampler, texture->image_view(), vk::ImageLayout::eShaderReadOnlyOptimal);
        vk::WriteDescriptorSet write_descriptor(m_set, index, 0, vk::DescriptorType::eCombinedImageSampler, image_info);
        API->m_device.updateDescriptorSets(write_descriptor, {});
        return *this;
    }
}// namespace Engine
